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
}

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
    // U64 whiteKing    = board.kings   & board.whites;
    // U64 blackKing    = board.kings   & board.blacks;
    // U64 allPieces    = board.whites | board.blacks;

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


    // --- 3) Gesamtsumme und Kappung --------------------------------------
    int score = materialScore;
    score = std::max(-SCORE_CLAMP, std::min(SCORE_CLAMP, score));
    return score;
}
