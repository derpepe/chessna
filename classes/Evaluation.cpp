#include "Evaluation.h"
#include "Board.h"
#include "MoveGenerator.h"
#include "Lib.h"

#include <algorithm>
#include <vector>

namespace {

// Centralized tuning constants for the evaluation.  Tweaking these values
// allows quick experimentation with different styles without digging into
// the evaluation logic.
namespace C {
    // Piece values
    constexpr int PAWN_VALUE   = 100;
    constexpr int KNIGHT_VALUE = 320;
    constexpr int BISHOP_VALUE = 330;
    constexpr int ROOK_VALUE   = 500;
    constexpr int QUEEN_VALUE  = 900;

    // Big score values
    constexpr int MATE_SCORE   = 1000000;
    constexpr int SCORE_CLAMP  = 30000;

    // Game phase & mobility tuning
    constexpr int PHASE_TOTAL  = 5180;   // total non-pawn material for phase
    constexpr int MOBILITY_CAP = 30;     // difference in legal moves considered
    constexpr int MOBILITY_WEIGHT = 2;   // cp per mobility point

    // Pawn structure
    constexpr int PAWN_CORE_MG = 12;     // pawn in the core centre
    constexpr int PAWN_EXT_MG  = 6;      // pawn in the extended centre
    constexpr int DOUBLED_PAWN = -12;
    constexpr int ISOLATED_PAWN = -15;
    constexpr int PASSED_MG[8] = {0,0,0,20,35,60,95,0};
    constexpr int PASSED_EG[8] = {0,0,0,25,45,75,120,0};

    // Knights
    constexpr int KNIGHT_CORE_MG = 30;
    constexpr int KNIGHT_CORE_EG = 20;
    constexpr int KNIGHT_EXT_MG  = 18;
    constexpr int KNIGHT_EXT_EG  = 10;
    constexpr int KNIGHT_EDGE_MG = -12;
    constexpr int KNIGHT_EDGE_EG = -8;
    constexpr int KNIGHT_CORNER_MG = -20;
    constexpr int KNIGHT_OUTPOST_MG = 25;
    constexpr int KNIGHT_SIXTH_MG = 10;

    // Bishops
    constexpr int BISHOP_PAIR_MG = 30;
    constexpr int BISHOP_PAIR_EG = 20;
    constexpr int BISHOP_OPEN_MG = 8;    // open diagonal
    constexpr int BISHOP_OPEN_EG = 10;
    constexpr int BISHOP_LONG_DIAG_MG = 10;

    // Rooks
    constexpr int ROOK_OPEN_MG = 18;
    constexpr int ROOK_OPEN_EG = 14;
    constexpr int ROOK_HALF_OPEN_MG = 10;
    constexpr int ROOK_SEVENTH_MG = 22;
    constexpr int ROOK_SEVENTH_EG = 16;
    constexpr int CONNECTED_ROOKS_MG = 12;

    // Queens
    constexpr int QUEEN_EARLY_CORE_MG = 6;
    constexpr int QUEEN_EARLY_EXT_MG = 3;
    constexpr int QUEEN_SEVENTH_MG = 12;
    constexpr int QUEEN_CORE_EG = 8;
    constexpr int QUEEN_EXT_EG = 4;

    // Kings
    constexpr int CASTLED_BONUS_MG = 20;
    constexpr int UNCASTLED_PENALTY_MG = -35; // after move 12 on starting square
    constexpr int SHIELD_BONUS_MG = 12;
    constexpr int SHIELD_MISSING_MG = -10;
    constexpr int KING_CORE_EG = 28;
    constexpr int KING_EXT_EG  = 16;

    // Aggressive extras
    constexpr int KING_PRESSURE_KNIGHT = 4;
    constexpr int KING_PRESSURE_BISHOP = 6;
    constexpr int KING_PRESSURE_ROOK   = 8;
    constexpr int KING_PRESSURE_QUEEN  = 8;
    constexpr int HOTSPOT_KNIGHT_MG = 10; // knights on f6/f3/e6/e3
}

inline int mirrorSquare(int sq) { return 63 - sq; }
inline unsigned long long fileMask(int file) { return 0x0101010101010101ULL << file; }

unsigned long long knightAttacks(int sq) {
    unsigned long long b = 0ULL;
    int r = Lib::getRank(sq);
    int f = Lib::getFile(sq);
    const int d[8][2] = {{1,2},{2,1},{-1,2},{-2,1},{1,-2},{2,-1},{-1,-2},{-2,-1}};
    for (auto &v : d) {
        int nr = r + v[1];
        int nf = f + v[0];
        if (nr>=0 && nr<8 && nf>=0 && nf<8) b |= 1ULL << (nr*8+nf);
    }
    return b;
}

unsigned long long kingAttacks(int sq) {
    unsigned long long b = 0ULL;
    int r = Lib::getRank(sq); int f = Lib::getFile(sq);
    for (int dr=-1; dr<=1; ++dr)
        for (int df=-1; df<=1; ++df) {
            if (!dr && !df) continue;
            int nr=r+dr, nf=f+df;
            if (nr>=0 && nr<8 && nf>=0 && nf<8) b |= 1ULL<<(nr*8+nf);
        }
    return b;
}

unsigned long long pawnAttacks(int sq, bool white) {
    unsigned long long b = 0ULL;
    int r = Lib::getRank(sq); int f = Lib::getFile(sq);
    if (white) {
        if (r<7 && f>0) b |= 1ULL<<((r+1)*8+f-1);
        if (r<7 && f<7) b |= 1ULL<<((r+1)*8+f+1);
    } else {
        if (r>0 && f>0) b |= 1ULL<<((r-1)*8+f-1);
        if (r>0 && f<7) b |= 1ULL<<((r-1)*8+f+1);
    }
    return b;
}

unsigned long long bishopAttacks(int sq, unsigned long long occ) {
    unsigned long long b=0ULL;
    const int dirs[4]={9,7,-9,-7};
    for (int d:dirs) {
        int s=sq;
        while (true) {
            int ns=s+d;
            if (ns<0 || ns>63) break;
            if (std::abs(Lib::getFile(ns)-Lib::getFile(s))!=1) break;
            b |= 1ULL<<ns;
            if (occ & (1ULL<<ns)) break;
            s=ns;
        }
    }
    return b;
}

unsigned long long rookAttacks(int sq, unsigned long long occ) {
    unsigned long long b=0ULL;
    const int dirs[4]={1,-1,8,-8};
    for (int d:dirs) {
        int s=sq;
        while (true) {
            int ns=s+d;
            if (ns<0 || ns>63) break;
            if ((d==1||d==-1) && Lib::getRank(ns)!=Lib::getRank(s)) break;
            b |= 1ULL<<ns;
            if (occ & (1ULL<<ns)) break;
            s=ns;
        }
    }
    return b;
}

} // namespace

// Evaluate a position from white's perspective in centipawns.
int Evaluation::evaluate(Board& board)
{
    MoveGenerator gen;
    std::vector<std::string> moves = gen.getAllMoves(board);
    // Deal with terminal positions early
    if (moves.empty()) {
        char us = board.getPlayerToMove();
        char them = us=='w'?'b':'w';
        unsigned long long king_bb = board.kings & ((us=='w')?board.whites:board.blacks);
        int ks = __builtin_ffsll(king_bb)-1;
        if (gen.isSquareAttacked(board, ks, them))
            return us=='w' ? -C::MATE_SCORE : C::MATE_SCORE;
        return 0; // stalemate or drawish repetition
    }

    // Material balance in centipawns
    int whiteMaterial =
        C::PAWN_VALUE   * __builtin_popcountll(board.pawns   & board.whites) +
        C::KNIGHT_VALUE * __builtin_popcountll(board.knights & board.whites) +
        C::BISHOP_VALUE * __builtin_popcountll(board.bishops & board.whites) +
        C::ROOK_VALUE   * __builtin_popcountll(board.rooks   & board.whites) +
        C::QUEEN_VALUE  * __builtin_popcountll(board.queens  & board.whites);
    int blackMaterial =
        C::PAWN_VALUE   * __builtin_popcountll(board.pawns   & board.blacks) +
        C::KNIGHT_VALUE * __builtin_popcountll(board.knights & board.blacks) +
        C::BISHOP_VALUE * __builtin_popcountll(board.bishops & board.blacks) +
        C::ROOK_VALUE   * __builtin_popcountll(board.rooks   & board.blacks) +
        C::QUEEN_VALUE  * __builtin_popcountll(board.queens  & board.blacks);
    int materialScore = whiteMaterial - blackMaterial;

    // Phase: blend middle-game and endgame components depending on material
    int nonPawn = (whiteMaterial + blackMaterial) -
                  C::PAWN_VALUE * __builtin_popcountll(board.pawns & (board.whites|board.blacks));
    double phase = std::max(0.0, std::min(1.0, nonPawn / (double)C::PHASE_TOTAL));

    int mg = 0, eg = 0; // middle-game and endgame scores
    unsigned long long occupied = board.whites | board.blacks;

    // Board masks used for piece-square reasoning
    const unsigned long long core = (1ULL<<27)|(1ULL<<28)|(1ULL<<35)|(1ULL<<36);
    const unsigned long long ext =
        (1ULL<<18)|(1ULL<<19)|(1ULL<<20)|(1ULL<<21)|
        (1ULL<<26)|(1ULL<<29)|
        (1ULL<<34)|(1ULL<<37)|
        (1ULL<<42)|(1ULL<<43)|(1ULL<<44)|(1ULL<<45)|
        (1ULL<<25)|(1ULL<<30)|
        (1ULL<<33)|(1ULL<<38);
    const unsigned long long edgeFiles = fileMask(0)|fileMask(1)|fileMask(6)|fileMask(7);
    const unsigned long long cornerMask =
        (1ULL<<0)|(1ULL<<7)|(1ULL<<56)|(1ULL<<63);

    // --- Pawns ------------------------------------------------------------
    auto evalPawns = [&](unsigned long long pawns, unsigned long long own, unsigned long long opp, bool white){
        while (pawns) {
            int sq = __builtin_ffsll(pawns)-1;
            unsigned long long bit = 1ULL<<sq;
            if (core & bit) mg += white?C::PAWN_CORE_MG:-C::PAWN_CORE_MG;
            else if (ext & bit) mg += white?C::PAWN_EXT_MG:-C::PAWN_EXT_MG;
            unsigned long long sameFile = pawns & fileMask(Lib::getFile(sq));
            if (__builtin_popcountll(sameFile) > 1) mg += white?C::DOUBLED_PAWN:-C::DOUBLED_PAWN;
            unsigned long long neigh = 0ULL;
            if (Lib::getFile(sq)>0) neigh |= pawns & fileMask(Lib::getFile(sq)-1);
            if (Lib::getFile(sq)<7) neigh |= pawns & fileMask(Lib::getFile(sq)+1);
            if (!neigh) mg += white?C::ISOLATED_PAWN:-C::ISOLATED_PAWN;
            unsigned long long frontSpan;
            if (white) {
                frontSpan = fileMask(Lib::getFile(sq));
                if (Lib::getFile(sq)>0) frontSpan |= fileMask(Lib::getFile(sq)-1);
                if (Lib::getFile(sq)<7) frontSpan |= fileMask(Lib::getFile(sq)+1);
                frontSpan &= ~((1ULL<<((Lib::getRank(sq)+1)*8))-1);
            } else {
                frontSpan = fileMask(Lib::getFile(sq));
                if (Lib::getFile(sq)>0) frontSpan |= fileMask(Lib::getFile(sq)-1);
                if (Lib::getFile(sq)<7) frontSpan |= fileMask(Lib::getFile(sq)+1);
                frontSpan &= ((1ULL<<(Lib::getRank(sq)*8))-1);
            }
            if ((board.pawns & opp & frontSpan) == 0) {
                int r = white ? Lib::getRank(sq) : 7-Lib::getRank(sq);
                mg += white?C::PASSED_MG[r]:-C::PASSED_MG[r];
                eg += white?C::PASSED_EG[r]:-C::PASSED_EG[r];
            }
            pawns &= pawns-1;
        }
    };
    evalPawns(board.pawns & board.whites, board.whites, board.blacks, true);
    evalPawns(board.pawns & board.blacks, board.blacks, board.whites, false);

    // --- Knights ----------------------------------------------------------
    auto evalKnights = [&](unsigned long long pieces, unsigned long long own, unsigned long long opp, bool white){
        while (pieces) {
            int sq=__builtin_ffsll(pieces)-1; unsigned long long bit=1ULL<<sq;
            if (core & bit) { mg += white?C::KNIGHT_CORE_MG:-C::KNIGHT_CORE_MG; eg += white?C::KNIGHT_CORE_EG:-C::KNIGHT_CORE_EG; }
            else if (ext & bit) { mg += white?C::KNIGHT_EXT_MG:-C::KNIGHT_EXT_MG; eg += white?C::KNIGHT_EXT_EG:-C::KNIGHT_EXT_EG; }
            if (edgeFiles & bit) { mg += white?C::KNIGHT_EDGE_MG:-C::KNIGHT_EDGE_MG; eg += white?C::KNIGHT_EDGE_EG:-C::KNIGHT_EDGE_EG; }
            if (cornerMask & bit) mg += white?C::KNIGHT_CORNER_MG:-C::KNIGHT_CORNER_MG;
            if ((pawnAttacks(sq,!white) & (board.pawns & opp))==0) mg += white?C::KNIGHT_OUTPOST_MG:-C::KNIGHT_OUTPOST_MG;
            int r = white ? Lib::getRank(sq) : Lib::getRank(mirrorSquare(sq));
            if (r==5) mg += white?C::KNIGHT_SIXTH_MG:-C::KNIGHT_SIXTH_MG;
            pieces&=pieces-1;
        }
    };
    evalKnights(board.knights & board.whites, board.whites, board.blacks, true);
    evalKnights(board.knights & board.blacks, board.blacks, board.whites, false);

    // --- Bishops ----------------------------------------------------------
    int whiteBish = __builtin_popcountll(board.bishops & board.whites);
    int blackBish = __builtin_popcountll(board.bishops & board.blacks);
    if (whiteBish>=2) { mg+=C::BISHOP_PAIR_MG; eg+=C::BISHOP_PAIR_EG; }
    if (blackBish>=2) { mg-=C::BISHOP_PAIR_MG; eg-=C::BISHOP_PAIR_EG; }
    auto evalBish = [&](unsigned long long pieces, bool white){
        while (pieces) {
            int sq=__builtin_ffsll(pieces)-1;
            unsigned long long attacks = bishopAttacks(sq, occupied);
            if (__builtin_popcountll(attacks)>=4) { mg+=white?C::BISHOP_OPEN_MG:-C::BISHOP_OPEN_MG; eg+=white?C::BISHOP_OPEN_EG:-C::BISHOP_OPEN_EG; }
            if (sq%9==0 || sq%7==0 || (sq-7)%9==0 || (sq-9)%7==0) mg+=white?C::BISHOP_LONG_DIAG_MG:-C::BISHOP_LONG_DIAG_MG;
            pieces&=pieces-1;
        }
    };
    evalBish(board.bishops & board.whites, true);
    evalBish(board.bishops & board.blacks, false);

    // --- Rooks ------------------------------------------------------------
    auto evalRooks = [&](unsigned long long pieces, unsigned long long own, unsigned long long opp, bool white){
        while (pieces) {
            int sq=__builtin_ffsll(pieces)-1;
            int file=Lib::getFile(sq);
            unsigned long long fm=fileMask(file);
            bool ownPawn=(board.pawns & own & fm)!=0;
            bool oppPawn=(board.pawns & opp & fm)!=0;
            if (!ownPawn && !oppPawn) { mg+=white?C::ROOK_OPEN_MG:-C::ROOK_OPEN_MG; eg+=white?C::ROOK_OPEN_EG:-C::ROOK_OPEN_EG; }
            else if (!ownPawn && oppPawn) { mg+=white?C::ROOK_HALF_OPEN_MG:-C::ROOK_HALF_OPEN_MG; }
            int r = white?Lib::getRank(sq):Lib::getRank(mirrorSquare(sq));
            if (r==6) { mg+=white?C::ROOK_SEVENTH_MG:-C::ROOK_SEVENTH_MG; eg+=white?C::ROOK_SEVENTH_EG:-C::ROOK_SEVENTH_EG; }
            pieces&=pieces-1;
        }
    };
    evalRooks(board.rooks & board.whites, board.whites, board.blacks, true);
    evalRooks(board.rooks & board.blacks, board.blacks, board.whites, false);

    // connected rooks
    auto connected = [&](unsigned long long pieces, bool white){
        if (__builtin_popcountll(pieces)>=2) {
            unsigned long long temp=pieces;
            int a=__builtin_ffsll(temp)-1; temp&=temp-1; int b=__builtin_ffsll(temp)-1;
            if (Lib::getFile(a)==Lib::getFile(b)) {
                unsigned long long between=0ULL; int f=Lib::getFile(a);
                int r1=Lib::getRank(a), r2=Lib::getRank(b);
                for(int r=std::min(r1,r2)+1;r<std::max(r1,r2);++r) between|=1ULL<<(r*8+f);
                if ((between & occupied)==0) mg+=white?C::CONNECTED_ROOKS_MG:-C::CONNECTED_ROOKS_MG;
            } else if (Lib::getRank(a)==Lib::getRank(b)) {
                unsigned long long between=0ULL; int r=Lib::getRank(a); int f1=Lib::getFile(a), f2=Lib::getFile(b);
                for(int f=std::min(f1,f2)+1;f<std::max(f1,f2);++f) between|=1ULL<<(r*8+f);
                if ((between & occupied)==0) mg+=white?C::CONNECTED_ROOKS_MG:-C::CONNECTED_ROOKS_MG;
            }
        }
    };
    connected(board.rooks & board.whites, true);
    connected(board.rooks & board.blacks, false);

    // --- Queens -----------------------------------------------------------
    auto evalQueens = [&](unsigned long long pieces, bool white){
        while (pieces) {
            int sq=__builtin_ffsll(pieces)-1; unsigned long long bit=1ULL<<sq;
            if (board.currentMove < 10) {
                if (core & bit) mg+=white?C::QUEEN_EARLY_CORE_MG:-C::QUEEN_EARLY_CORE_MG;
                else if (ext & bit) mg+=white?C::QUEEN_EARLY_EXT_MG:-C::QUEEN_EARLY_EXT_MG;
            }
            int r = white?Lib::getRank(sq):Lib::getRank(mirrorSquare(sq));
            if (r==6) mg+=white?C::QUEEN_SEVENTH_MG:-C::QUEEN_SEVENTH_MG;
            if (core & bit) eg+=white?C::QUEEN_CORE_EG:-C::QUEEN_CORE_EG;
            else if (ext & bit) eg+=white?C::QUEEN_EXT_EG:-C::QUEEN_EXT_EG;
            pieces&=pieces-1;
        }
    };
    evalQueens(board.queens & board.whites, true);
    evalQueens(board.queens & board.blacks, false);

    // --- King -------------------------------------------------------------
    auto evalKing = [&](unsigned long long king, unsigned long long own, bool white){
        int sq=__builtin_ffsll(king)-1; unsigned long long bit=1ULL<<sq;
        if ((white && (sq==6 || sq==2)) || (!white && (sq==62 || sq==58)))
            mg+=white?C::CASTLED_BONUS_MG:-C::CASTLED_BONUS_MG;
        if (board.currentMove>12 && (sq==4 || sq==3 || sq==60 || sq==59))
            mg+=white?C::UNCASTLED_PENALTY_MG:-C::UNCASTLED_PENALTY_MG;
        if (white) {
            if (sq==6) {
                mg += (board.pawns & own & (1ULL<<14))?C::SHIELD_BONUS_MG:C::SHIELD_MISSING_MG;
                mg += (board.pawns & own & (1ULL<<13))?C::SHIELD_BONUS_MG:C::SHIELD_MISSING_MG;
                mg += (board.pawns & own & (1ULL<<15))?C::SHIELD_BONUS_MG:C::SHIELD_MISSING_MG;
            } else if (sq==2) {
                mg += (board.pawns & own & (1ULL<<10))?C::SHIELD_BONUS_MG:C::SHIELD_MISSING_MG;
                mg += (board.pawns & own & (1ULL<<11))?C::SHIELD_BONUS_MG:C::SHIELD_MISSING_MG;
                mg += (board.pawns & own & (1ULL<<9))?C::SHIELD_BONUS_MG:C::SHIELD_MISSING_MG;
            }
        } else {
            if (sq==62) {
                mg -= (board.pawns & own & (1ULL<<46))?C::SHIELD_BONUS_MG:C::SHIELD_MISSING_MG;
                mg -= (board.pawns & own & (1ULL<<45))?C::SHIELD_BONUS_MG:C::SHIELD_MISSING_MG;
                mg -= (board.pawns & own & (1ULL<<47))?C::SHIELD_BONUS_MG:C::SHIELD_MISSING_MG;
            } else if (sq==58) {
                mg -= (board.pawns & own & (1ULL<<50))?C::SHIELD_BONUS_MG:C::SHIELD_MISSING_MG;
                mg -= (board.pawns & own & (1ULL<<51))?C::SHIELD_BONUS_MG:C::SHIELD_MISSING_MG;
                mg -= (board.pawns & own & (1ULL<<49))?C::SHIELD_BONUS_MG:C::SHIELD_MISSING_MG;
            }
        }
        if (core & bit) eg+=white?C::KING_CORE_EG:-C::KING_CORE_EG; else if (ext & bit) eg+=white?C::KING_EXT_EG:-C::KING_EXT_EG;
    };
    evalKing(board.kings & board.whites, board.whites, true);
    evalKing(board.kings & board.blacks, board.blacks, false);

    // mobility
    auto countMoves = [&](Board b, char side){ b.playerToMove=side; return (int)gen.getAllMoves(b).size(); };
    int mobW=countMoves(board,'w');
    int mobB=countMoves(board,'b');
    int diff=std::max(-C::MOBILITY_CAP,std::min(C::MOBILITY_CAP,mobW-mobB));
    mg += diff*C::MOBILITY_WEIGHT;

    // king pressure: count attackers around opponent king
    auto kingPressure = [&](unsigned long long attackerPieces, bool white){
        unsigned long long king = board.kings & (white?board.blacks:board.whites);
        int ksq = __builtin_ffsll(king)-1;
        unsigned long long zone = kingAttacks(ksq) | (1ULL<<ksq);
        unsigned long long attacks=0ULL;
        unsigned long long bits = board.knights & attackerPieces;
        while(bits){int s=__builtin_ffsll(bits)-1; attacks|=knightAttacks(s); bits&=bits-1;}
        mg += (int)__builtin_popcountll(attacks & zone)*(white?C::KING_PRESSURE_KNIGHT:-C::KING_PRESSURE_KNIGHT);
        attacks=0ULL; bits = board.bishops & attackerPieces;
        while(bits){int s=__builtin_ffsll(bits)-1; attacks|=bishopAttacks(s,occupied); bits&=bits-1;}
        mg += (int)__builtin_popcountll(attacks & zone)*(white?C::KING_PRESSURE_BISHOP:-C::KING_PRESSURE_BISHOP);
        attacks=0ULL; bits = board.rooks & attackerPieces;
        while(bits){int s=__builtin_ffsll(bits)-1; attacks|=rookAttacks(s,occupied); bits&=bits-1;}
        mg += (int)__builtin_popcountll(attacks & zone)*(white?C::KING_PRESSURE_ROOK:-C::KING_PRESSURE_ROOK);
        attacks=0ULL; bits = board.queens & attackerPieces;
        while(bits){int s=__builtin_ffsll(bits)-1; attacks|=(rookAttacks(s,occupied)|bishopAttacks(s,occupied)); bits&=bits-1;}
        mg += (int)__builtin_popcountll(attacks & zone)*(white?C::KING_PRESSURE_QUEEN:-C::KING_PRESSURE_QUEEN);
    };
    kingPressure(board.whites,true);
    kingPressure(board.blacks,false);

    auto hotspot=[&](int sq){ if(board.knights & (1ULL<<sq)){ if(board.whites & (1ULL<<sq)) mg+=C::HOTSPOT_KNIGHT_MG; else mg-=C::HOTSPOT_KNIGHT_MG; } };
    hotspot(Lib::getBitnumFromCoordinates("f6"));
    hotspot(Lib::getBitnumFromCoordinates("f3"));
    hotspot(Lib::getBitnumFromCoordinates("e6"));
    hotspot(Lib::getBitnumFromCoordinates("e3"));

    // Blend middle-game and endgame contributions
    int pstScore = (int)(mg * phase + eg * (1.0 - phase));
    int score = materialScore + pstScore;
    if (score > C::SCORE_CLAMP) score = C::SCORE_CLAMP;
    if (score < -C::SCORE_CLAMP) score = -C::SCORE_CLAMP;
    return score;
}
