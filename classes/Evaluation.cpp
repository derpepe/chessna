#include "Evaluation.h"
#include "Board.h"
#include "MoveGenerator.h"
#include "Lib.h"

#include <algorithm>
#include <vector>
#include <string>
#include <cstdint>

// Die gesamte Bewertungslogik ist in diesem Modul gekapselt.  Sämtliche
// Zahlenwerte sind als Konstanten hinterlegt, sodass sie später leicht
// getuned werden können, ohne den Code selbst zu ändern.
namespace {

using U64 = unsigned long long;

namespace C {
    // Grundlegende Figurenwerte in Zentipawns
    constexpr int PAWN_VALUE   = 100;
    constexpr int KNIGHT_VALUE = 320;
    constexpr int BISHOP_VALUE = 330;
    constexpr int ROOK_VALUE   = 500;
    constexpr int QUEEN_VALUE  = 900;

    // Endwerte für terminale Stellungen
    constexpr int MATE_SCORE  = 1000000;
    constexpr int SCORE_CLAMP = 30000;

    // Phasenberechnung (Nicht-Bauern-Material beider Seiten)
    constexpr double MAX_PHASE = 6400.0;

    // Konstanten für Zentrums- und Aktivitätsboni
    constexpr int KNIGHT_CORE_MG = 20;   // Springer im Kernzentrum (Mittelspiel)
    constexpr int KNIGHT_CORE_EG = 15;   // Springer im Kernzentrum (Endspiel)
    constexpr int KNIGHT_EXT_MG  = 10;   // Springer im erweiterten Zentrum (Mittelspiel)
    constexpr int KNIGHT_EXT_EG  = 7;    // Springer im erweiterten Zentrum (Endspiel)
    constexpr int KNIGHT_EDGE_MG = -10;  // Springer auf Randfeldern (Mittelspiel)
    constexpr int KNIGHT_EDGE_EG = -6;   // Springer auf Randfeldern (Endspiel)

    constexpr int BISHOP_CORE_MG = 10;   // Läufer im Kernzentrum
    constexpr int BISHOP_EXT_MG  = 6;    // Läufer im erweiterten Zentrum

    constexpr int QUEEN_CORE_MG  = 5;    // Dame im Kernzentrum (Mittelspiel)
    constexpr int QUEEN_CORE_EG  = 8;    // Dame im Kernzentrum (Endspiel)
    constexpr int QUEEN_EXT_MG   = 3;    // Dame im erweiterten Zentrum

    constexpr int KING_CORE_EG   = 30;   // König im Kernzentrum (Endspiel)
    constexpr int KING_EXT_EG    = 16;   // König im erweiterten Zentrum (Endspiel)

    // Bauernstruktur
    constexpr int DOUBLED_PAWN_PENALTY   = -10;   // Doppelbauer
    constexpr int ISOLATED_PAWN_PENALTY  = -15;   // Isolierter Bauer
    constexpr int BACKWARD_PAWN_PENALTY  = -12;   // Rückändiger Bauer
    constexpr int PASSED_BONUS[8]        = {0,0,0,15,30,45,70,0}; // Rangabhängige Boni
    constexpr int BLOCKED_PASSED_PENALTY = -10;   // Blockierter Freibauer
    constexpr int CONNECTED_PASSED_BONUS = 15;    // Zusätzlicher Bonus für verbundene Freibauern

    // Königssicherheit
    constexpr int CASTLED_BONUS        = 15;  // König hat rochiert
    constexpr int LATE_UNCASTLED       = -20; // Nach Zug 12 nicht rochiert
    constexpr int OPEN_FILE_NEAR_KING  = -12; // Offene/halb-offene Linie neben dem König

    // Rook & Co.
    constexpr int ROOK_OPEN_FILE_BONUS      = 15;
    constexpr int ROOK_HALF_OPEN_FILE_BONUS = 7;
    constexpr int ROOK_SEVENTH_BONUS        = 20;
    constexpr int BISHOP_PAIR_BONUS         = 25;

    // Mobilität
    constexpr int MOBILITY_WEIGHT = 1;   // 1 cp pro Zug Differenz
    constexpr int MOBILITY_CAP    = 30;  // maximal ±30 cp
}

// Hilfsfunktionen ----------------------------------------------------------
inline int mirrorSquare(int sq) { return 63 - sq; }
inline U64 fileMask(int f) { return 0x0101010101010101ULL << f; }
inline int popLSB(U64 &b) { int s = __builtin_ctzll(b); b &= b - 1; return s; }
inline U64 pawnAttacks(int sq, bool white) {
    U64 b=0ULL;
    int r=Lib::getRank(sq); int f=Lib::getFile(sq);
    if (white) {
        if (r<7 && f>0) b|=1ULL<<((r+1)*8+f-1);
        if (r<7 && f<7) b|=1ULL<<((r+1)*8+f+1);
    } else {
        if (r>0 && f>0) b|=1ULL<<((r-1)*8+f-1);
        if (r>0 && f<7) b|=1ULL<<((r-1)*8+f+1);
    }
    return b;
}
// Indizes wichtiger Felder
constexpr int SQ_A1 = 0, SQ_C1 = 2, SQ_D1 = 3, SQ_E1 = 4, SQ_G1 = 6;
constexpr int SQ_C8 = 58, SQ_D8 = 59, SQ_E8 = 60, SQ_G8 = 62;

// Bitboards für Zentrum und Rand
constexpr U64 CORE_CENTER = (1ULL<<27)|(1ULL<<28)|(1ULL<<35)|(1ULL<<36);
constexpr U64 EXT_CENTER  = (1ULL<<18)|(1ULL<<19)|(1ULL<<20)|(1ULL<<21)|
                            (1ULL<<26)|(1ULL<<27)|(1ULL<<28)|(1ULL<<29)|
                            (1ULL<<34)|(1ULL<<35)|(1ULL<<36)|(1ULL<<37)|
                            (1ULL<<42)|(1ULL<<43)|(1ULL<<44)|(1ULL<<45);
constexpr U64 EDGE_FILES  = 0x8181818181818181ULL; // a- und h-Linie

} // namespace

// ---------------------------------------------------------------------------
// Hauptfunktion: Bewertet die aktuelle Stellung aus Sicht von Weiß.
// ---------------------------------------------------------------------------
int Evaluation::evaluate(Board& board)
{
    using namespace C;
    MoveGenerator gen;

    // --- 1) Terminale Zustände -------------------------------------------
    std::vector<std::string> moves = board.getAllMoves();
    if (moves.empty()) {
        U64 kingBB = board.kings & (board.playerToMove == 'w' ? board.whites : board.blacks);
        int kingSq = __builtin_ctzll(kingBB);
        char opponent = (board.playerToMove == 'w') ? 'b' : 'w';
        if (gen.isSquareAttacked(board, kingSq, opponent)) {
            return (board.playerToMove == 'w') ? -MATE_SCORE : MATE_SCORE;
        }
        return 0; // Patt
    }

    // --- 2) Material und Spielphase --------------------------------------
    U64 whitePawns   = board.pawns   & board.whites;
    U64 blackPawns   = board.pawns   & board.blacks;
    U64 whiteKnights = board.knights & board.whites;
    U64 blackKnights = board.knights & board.blacks;
    U64 whiteBishops = board.bishops & board.whites;
    U64 blackBishops = board.bishops & board.blacks;
    U64 whiteRooks   = board.rooks   & board.whites;
    U64 blackRooks   = board.rooks   & board.blacks;
    U64 whiteQueens  = board.queens  & board.whites;
    U64 blackQueens  = board.queens  & board.blacks;
    U64 whiteKing    = board.kings   & board.whites;
    U64 blackKing    = board.kings   & board.blacks;
    U64 allPieces    = board.whites | board.blacks;

    int whiteMaterial = __builtin_popcountll(whitePawns)   * PAWN_VALUE +
                        __builtin_popcountll(whiteKnights) * KNIGHT_VALUE +
                        __builtin_popcountll(whiteBishops) * BISHOP_VALUE +
                        __builtin_popcountll(whiteRooks)   * ROOK_VALUE +
                        __builtin_popcountll(whiteQueens)  * QUEEN_VALUE;

    int blackMaterial = __builtin_popcountll(blackPawns)   * PAWN_VALUE +
                        __builtin_popcountll(blackKnights) * KNIGHT_VALUE +
                        __builtin_popcountll(blackBishops) * BISHOP_VALUE +
                        __builtin_popcountll(blackRooks)   * ROOK_VALUE +
                        __builtin_popcountll(blackQueens)  * QUEEN_VALUE;

    int materialScore = whiteMaterial - blackMaterial;

    int nonPawnMaterial = (whiteMaterial - __builtin_popcountll(whitePawns) * PAWN_VALUE) +
                          (blackMaterial - __builtin_popcountll(blackPawns) * PAWN_VALUE);

    double phase = std::max(0.0, std::min(1.0, nonPawnMaterial / MAX_PHASE));
    double mg = phase;            // Anteil Mittelspiel
    double eg = 1.0 - phase;      // Anteil Endspiel

    // Vorberechnung: sind auf einer Linie eigene / gegnerische Bauern?
    bool whitePawnFile[8] = {false};
    bool blackPawnFile[8] = {false};
    for (int f = 0; f < 8; ++f) {
        U64 mask = fileMask(f);
        if (whitePawns & mask) whitePawnFile[f] = true;
        if (blackPawns & mask) blackPawnFile[f] = true;
    }

    // --- 2.1 Zentrums- und Aktivitätsboni ---------------------------------
    int centerMG = 0, centerEG = 0;

    U64 tmp = whiteKnights;
    while (tmp) {
        int sq = popLSB(tmp);
        U64 bit = 1ULL << sq;
        if      (bit & CORE_CENTER) { centerMG += KNIGHT_CORE_MG; centerEG += KNIGHT_CORE_EG; }
        else if (bit & EXT_CENTER)  { centerMG += KNIGHT_EXT_MG;  centerEG += KNIGHT_EXT_EG;  }
        if (bit & EDGE_FILES)       { centerMG += KNIGHT_EDGE_MG; centerEG += KNIGHT_EDGE_EG; }
    }
    tmp = blackKnights;
    while (tmp) {
        int sq = popLSB(tmp);
        int msq = mirrorSquare(sq);
        U64 bit = 1ULL << msq;
        if      (bit & CORE_CENTER) { centerMG -= KNIGHT_CORE_MG; centerEG -= KNIGHT_CORE_EG; }
        else if (bit & EXT_CENTER)  { centerMG -= KNIGHT_EXT_MG;  centerEG -= KNIGHT_EXT_EG;  }
        if (bit & EDGE_FILES)       { centerMG -= KNIGHT_EDGE_MG; centerEG -= KNIGHT_EDGE_EG; }
    }

    tmp = whiteBishops;
    while (tmp) {
        int sq = popLSB(tmp);
        U64 bit = 1ULL << sq;
        if      (bit & CORE_CENTER) { centerMG += BISHOP_CORE_MG; centerEG += BISHOP_CORE_MG; }
        else if (bit & EXT_CENTER)  { centerMG += BISHOP_EXT_MG;  centerEG += BISHOP_EXT_MG;  }
    }
    tmp = blackBishops;
    while (tmp) {
        int sq = popLSB(tmp);
        int msq = mirrorSquare(sq);
        U64 bit = 1ULL << msq;
        if      (bit & CORE_CENTER) { centerMG -= BISHOP_CORE_MG; centerEG -= BISHOP_CORE_MG; }
        else if (bit & EXT_CENTER)  { centerMG -= BISHOP_EXT_MG;  centerEG -= BISHOP_EXT_MG;  }
    }

    tmp = whiteQueens;
    while (tmp) {
        int sq = popLSB(tmp);
        U64 bit = 1ULL << sq;
        if      (bit & CORE_CENTER) { centerMG += QUEEN_CORE_MG; centerEG += QUEEN_CORE_EG; }
        else if (bit & EXT_CENTER)  { centerMG += QUEEN_EXT_MG; centerEG += QUEEN_EXT_MG; }
    }
    tmp = blackQueens;
    while (tmp) {
        int sq = popLSB(tmp);
        int msq = mirrorSquare(sq);
        U64 bit = 1ULL << msq;
        if      (bit & CORE_CENTER) { centerMG -= QUEEN_CORE_MG; centerEG -= QUEEN_CORE_EG; }
        else if (bit & EXT_CENTER)  { centerMG -= QUEEN_EXT_MG; centerEG -= QUEEN_EXT_MG; }
    }

    int whiteKingSq = __builtin_ctzll(whiteKing);
    int blackKingSq = __builtin_ctzll(blackKing);
    if      ((1ULL << whiteKingSq) & CORE_CENTER) { centerEG += KING_CORE_EG; }
    else if ((1ULL << whiteKingSq) & EXT_CENTER)  { centerEG += KING_EXT_EG; }
    int msq = mirrorSquare(blackKingSq);
    if      ((1ULL << msq) & CORE_CENTER) { centerEG -= KING_CORE_EG; }
    else if ((1ULL << msq) & EXT_CENTER)  { centerEG -= KING_EXT_EG; }

    // --- 2.2 Bauernstruktur -----------------------------------------------
    int pawnScore = 0;
    // Doppelbauern je Linie
    for (int f = 0; f < 8; ++f) {
        int wc = __builtin_popcountll(whitePawns & fileMask(f));
        if (wc > 1) pawnScore += (wc - 1) * DOUBLED_PAWN_PENALTY;
        int bc = __builtin_popcountll(blackPawns & fileMask(f));
        if (bc > 1) pawnScore -= (bc - 1) * DOUBLED_PAWN_PENALTY;
    }

    U64 passedWhite = 0ULL, passedBlack = 0ULL;

    // Bewertung weißer Bauern
    tmp = whitePawns;
    while (tmp) {
        int sq = popLSB(tmp);
        int file = Lib::getFile(sq);
        int rank = Lib::getRank(sq);
        // Isoliert?
        bool left  = (file > 0) && whitePawnFile[file-1];
        bool right = (file < 7) && whitePawnFile[file+1];
        if (!left && !right) pawnScore += ISOLATED_PAWN_PENALTY;
        // Rückändig?
        bool leftAhead  = (file > 0) && (whitePawns & fileMask(file-1) & (~0ULL << ((rank+1)*8)));
        bool rightAhead = (file < 7) && (whitePawns & fileMask(file+1) & (~0ULL << ((rank+1)*8)));
        U64 front = 1ULL << (sq + 8);
        bool enemyCtrl = (pawnAttacks(sq + 8, false) & blackPawns) != 0ULL;
        if (!leftAhead && !rightAhead && !blackPawnFile[file] && !(allPieces & front) && enemyCtrl)
            pawnScore += BACKWARD_PAWN_PENALTY;
        // Freibauer?
        U64 inFront = (~0ULL) << ((rank+1)*8);
        U64 blockers = blackPawns & inFront & (fileMask(file) |
                       (file > 0 ? fileMask(file-1) : 0ULL) |
                       (file < 7 ? fileMask(file+1) : 0ULL));
        if (!blockers) {
            passedWhite |= 1ULL << sq;
            int r = rank + 1; // Rang aus Weiß-Sicht
            pawnScore += PASSED_BONUS[r];
            if (allPieces & front) pawnScore += BLOCKED_PASSED_PENALTY;
        }
    }

    // Bewertung schwarzer Bauern
    tmp = blackPawns;
    while (tmp) {
        int sq = popLSB(tmp);
        int file = Lib::getFile(sq);
        int rank = Lib::getRank(sq);
        bool left  = (file > 0) && blackPawnFile[file-1];
        bool right = (file < 7) && blackPawnFile[file+1];
        if (!left && !right) pawnScore -= ISOLATED_PAWN_PENALTY;
        bool leftAhead  = (file > 0) && (blackPawns & fileMask(file-1) & ((1ULL << (rank*8)) - 1ULL));
        bool rightAhead = (file < 7) && (blackPawns & fileMask(file+1) & ((1ULL << (rank*8)) - 1ULL));
        U64 front = 1ULL << (sq - 8);
        bool enemyCtrl = (pawnAttacks(sq - 8, true) & whitePawns) != 0ULL;
        if (!leftAhead && !rightAhead && !whitePawnFile[file] && !(allPieces & front) && enemyCtrl)
            pawnScore -= BACKWARD_PAWN_PENALTY;
        U64 inFront = (1ULL << (rank*8)) - 1ULL;
        U64 blockers = whitePawns & inFront & (fileMask(file) |
                       (file > 0 ? fileMask(file-1) : 0ULL) |
                       (file < 7 ? fileMask(file+1) : 0ULL));
        if (!blockers) {
            passedBlack |= 1ULL << sq;
            int r = 8 - (rank + 1);
            pawnScore -= PASSED_BONUS[r];
            if (allPieces & front) pawnScore -= BLOCKED_PASSED_PENALTY;
        }
    }

    // Verbundene Freibauern
    U64 whiteHigh = passedWhite & (~0ULL << (4*8)); // mind. Rang 5
    tmp = whiteHigh;
    while (tmp) {
        int sq = popLSB(tmp);
        int f = Lib::getFile(sq);
        if (f < 7 && (whiteHigh & fileMask(f+1))) pawnScore += CONNECTED_PASSED_BONUS;
    }
    U64 blackHigh = passedBlack & ((1ULL << (4*8)) - 1ULL); // mind. Rang 5 aus Schwarzsicht
    tmp = blackHigh;
    while (tmp) {
        int sq = popLSB(tmp);
        int f = Lib::getFile(sq);
        if (f < 7 && (blackHigh & fileMask(f+1))) pawnScore -= CONNECTED_PASSED_BONUS;
    }

    // --- 2.3 Königssicherheit ------------------------------------------
    int kingMG = 0, kingEG = 0;
    // Rochiert?
    if (whiteKingSq == SQ_G1 || whiteKingSq == SQ_C1) kingMG += CASTLED_BONUS;
    if (blackKingSq == SQ_G8 || blackKingSq == SQ_C8) kingMG -= CASTLED_BONUS;
    // Nach Zug 12 nicht rochiert?
    if (board.currentMove > 12) {
        if (whiteKingSq == SQ_E1 || whiteKingSq == SQ_D1) kingMG += LATE_UNCASTLED;
        if (blackKingSq == SQ_E8 || blackKingSq == SQ_D8) kingMG -= LATE_UNCASTLED;
    }
    // Offene/halb-offene Linien neben dem König
    int kf = Lib::getFile(whiteKingSq);
    for (int df = -1; df <= 1; ++df) {
        int f = kf + df;
        if (f >= 0 && f < 8 && !whitePawnFile[f]) kingMG += OPEN_FILE_NEAR_KING;
    }
    kf = Lib::getFile(blackKingSq);
    for (int df = -1; df <= 1; ++df) {
        int f = kf + df;
        if (f >= 0 && f < 8 && !blackPawnFile[f]) kingMG -= OPEN_FILE_NEAR_KING;
    }

    // --- 2.4 Rooks & Läuferpaar ---------------------------------------
    int rookScore = 0;
    tmp = whiteRooks;
    while (tmp) {
        int sq = popLSB(tmp);
        int f = Lib::getFile(sq);
        if (!whitePawnFile[f] && !blackPawnFile[f])      rookScore += ROOK_OPEN_FILE_BONUS;
        else if (!whitePawnFile[f] && blackPawnFile[f])  rookScore += ROOK_HALF_OPEN_FILE_BONUS;
        if (Lib::getRank(sq) == 6) rookScore += ROOK_SEVENTH_BONUS; // 7. Reihe
    }
    tmp = blackRooks;
    while (tmp) {
        int sq = popLSB(tmp);
        int f = Lib::getFile(sq);
        if (!whitePawnFile[f] && !blackPawnFile[f])      rookScore -= ROOK_OPEN_FILE_BONUS;
        else if (!blackPawnFile[f] && whitePawnFile[f])  rookScore -= ROOK_HALF_OPEN_FILE_BONUS;
        if (Lib::getRank(mirrorSquare(sq)) == 6) rookScore -= ROOK_SEVENTH_BONUS; // 2. Reihe aus Schwarzsicht
    }
    if (__builtin_popcountll(whiteBishops) >= 2) rookScore += BISHOP_PAIR_BONUS;
    if (__builtin_popcountll(blackBishops) >= 2) rookScore -= BISHOP_PAIR_BONUS;

    // --- 2.5 Mobilität --------------------------------------------------
    Board tmpBoard(board);
    tmpBoard.playerToMove = 'w';
    int whiteMoves = tmpBoard.getAllMoves().size();
    tmpBoard.playerToMove = 'b';
    int blackMoves = tmpBoard.getAllMoves().size();
    int mobility = whiteMoves - blackMoves;
    mobility = std::max(-MOBILITY_CAP, std::min(MOBILITY_CAP, mobility));
    int mobilityScore = mobility * MOBILITY_WEIGHT;

    // --- 3) Phasenmischung -----------------------------------------------
    int centerScore = static_cast<int>(centerMG * mg + centerEG * eg);
    int kingScore   = static_cast<int>(kingMG   * mg + kingEG   * eg);

    // --- 4) Gesamtsumme und Kappung --------------------------------------
    int score = materialScore + centerScore + pawnScore + rookScore + kingScore + mobilityScore;
    score = std::max(-SCORE_CLAMP, std::min(SCORE_CLAMP, score));
    return score;
}
