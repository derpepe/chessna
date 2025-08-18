#include "Evaluation.h"
#include "Board.h"
#include "MoveGenerator.h"
#include "Lib.h"

#include <algorithm>
#include <array>
#include <cstdint>

namespace {

using U64 = unsigned long long;

inline int mirror_rank(int sq) {
    int file = Lib::getFile(sq);
    int rank = Lib::getRank(sq);
    int mrank = 7 - rank;
    return mrank * 8 + file;
}

constexpr int PAWN_MG  = 100, PAWN_EG  = 120;
constexpr int KNIGHT_MG= 320, KNIGHT_EG= 300;
constexpr int BISHOP_MG= 330, BISHOP_EG= 330;
constexpr int ROOK_MG  = 500, ROOK_EG  = 520;
constexpr int QUEEN_MG = 900, QUEEN_EG = 900;

constexpr int BISHOP_PAIR_BONUS = 35;
constexpr int ROOK_OPEN_FILE    = 20;
constexpr int ROOK_SEMI_OPEN    = 10;

constexpr int ISOLATED_PAWN_PEN = 15;
constexpr int DOUBLED_PAWN_PEN  = 12;

constexpr int PASSED_PAWN_BONUS[8] = {0,10,20,35,55,80,120,0};
constexpr int SCORE_CLAMP = 20000;

const std::array<int,64> PAWN_PST_MG = {{
      0,  0,  0,  0,  0,  0,  0,  0,
     50, 50, 50, 50, 50, 50, 50, 50,
     10, 10, 20, 30, 30, 20, 10, 10,
      5,  5, 10, 25, 25, 10,  5,  5,
      2,  2,  5, 12, 12,  5,  2,  2,
      0,  0,  0, 10, 10,  0,  0,  0,
      0,  0,  0, -8, -8,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0
}};
const std::array<int,64> PAWN_PST_EG = {{
      0,  0,  0,  0,  0,  0,  0,  0,
     20, 20, 20, 25, 25, 20, 20, 20,
     15, 15, 18, 22, 22, 18, 15, 15,
     10, 10, 12, 15, 15, 12, 10, 10,
      8,  8, 10, 12, 12, 10,  8,  8,
      5,  5,  7, 10, 10,  7,  5,  5,
      2,  2,  3,  5,  5,  3,  2,  2,
      0,  0,  0,  0,  0,  0,  0,  0
}};
const std::array<int,64> KNIGHT_PST_MG = {{
    -50,-40,-30,-30,-30,-30,-40,-50,
    -40,-20,  0,  0,  0,  0,-20,-40,
    -30,  0, 10, 15, 15, 10,  0,-30,
    -30,  5, 15, 20, 20, 15,  5,-30,
    -30,  0, 15, 20, 20, 15,  0,-30,
    -30,  5, 10, 15, 15, 10,  5,-30,
    -40,-20,  0,  5,  5,  0,-20,-40,
    -50,-40,-30,-30,-30,-30,-40,-50
}};
const std::array<int,64> KNIGHT_PST_EG = KNIGHT_PST_MG;
const std::array<int,64> BISHOP_PST_MG = {{
    -20,-10,-10,-10,-10,-10,-10,-20,
    -10,  5,  0,  0,  0,  0,  5,-10,
    -10, 10, 10, 10, 10, 10, 10,-10,
    -10,  0, 10, 15, 15, 10,  0,-10,
    -10,  0, 10, 15, 15, 10,  0,-10,
    -10, 10, 10, 10, 10, 10, 10,-10,
    -10,  5,  0,  0,  0,  0,  5,-10,
    -20,-10,-10,-10,-10,-10,-10,-20
}};
const std::array<int,64> BISHOP_PST_EG = BISHOP_PST_MG;
const std::array<int,64> ROOK_PST_MG = {{
      0,  0,  5, 10, 10,  5,  0,  0,
     -5,  0,  0,  0,  0,  0,  0, -5,
     -5,  0,  0,  0,  0,  0,  0, -5,
     -5,  0,  0,  5,  5,  0,  0, -5,
     -5,  0,  0,  5,  5,  0,  0, -5,
     -5,  0,  0,  0,  0,  0,  0, -5,
      5, 10, 10, 10, 10, 10, 10,  5,
      0,  0,  5, 10, 10,  5,  0,  0
}};
const std::array<int,64> ROOK_PST_EG = ROOK_PST_MG;
const std::array<int,64> QUEEN_PST_MG = {{
    -20,-10,-10, -5, -5,-10,-10,-20,
    -10,  0,  0,  0,  0,  5,  0,-10,
    -10,  0,  5,  5,  5,  5,  0,-10,
     -5,  0,  5,  5,  5,  5,  0, -5,
      0,  0,  5,  5,  5,  5,  0, -5,
    -10,  5,  5,  5,  5,  5,  0,-10,
    -10,  0,  5,  0,  0,  0,  0,-10,
    -20,-10,-10, -5, -5,-10,-10,-20
}};
const std::array<int,64> QUEEN_PST_EG = QUEEN_PST_MG;
const std::array<int,64> KING_PST_MG = {{
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-35,-35,-40,-40,-35,-35,-30,
    -20,-25,-25,-30,-30,-25,-25,-20,
    -10,-10,-10,-10,-10,-10,-10,-10,
     20, 20,  5,  0,  0,  5, 20, 20,
     20, 30, 10,  0,  0, 10, 30, 20
}};
const std::array<int,64> KING_PST_EG = {{
    -50,-40,-30,-20,-20,-30,-40,-50,
    -40,-20,-10,  0,  0,-10,-20,-40,
    -30,-10, 10, 20, 20, 10,-10,-30,
    -20,  0, 20, 30, 30, 20,  0,-20,
    -20,  0, 20, 30, 30, 20,  0,-20,
    -30,-10, 10, 20, 20, 10,-10,-30,
    -40,-20,-10,  0,  0,-10,-20,-40,
    -50,-40,-30,-20,-20,-30,-40,-50
}};

struct Score {
    int mg = 0;
    int eg = 0;
    void addMG(int v){ mg += v; }
    void addEG(int v){ eg += v; }
    int blend(int phase) const {
        return (mg * phase + eg * (24 - phase)) / 24;
    }
};

int game_phase(const Board& b) {
    int n = __builtin_popcountll(b.knights);
    int bsh = __builtin_popcountll(b.bishops);
    int r = __builtin_popcountll(b.rooks);
    int q = __builtin_popcountll(b.queens);
    int phase = 24;
    phase -= n * 1;
    phase -= bsh * 1;
    phase -= r * 2;
    phase -= q * 4;
    if (phase < 0) phase = 0;
    if (phase > 24) phase = 24;
    return phase;
}

int doubled_pawn_penalty(const Board& b, bool white) {
    unsigned long long pawns = (b.pawns & (white ? b.whites : b.blacks));
    int pen = 0;
    for (int file = 0; file < 8; ++file) {
        unsigned long long fileMask = 0ULL;
        for (int r = 0; r < 8; ++r) fileMask |= (1ULL << (r*8 + file));
        int cnt = __builtin_popcountll(pawns & fileMask);
        if (cnt >= 2) pen += (cnt - 1) * DOUBLED_PAWN_PEN;
    }
    return pen;
}

int isolated_pawn_penalty(const Board& b, bool white) {
    unsigned long long pawns = (b.pawns & (white ? b.whites : b.blacks));
    int pen = 0;
    for (unsigned long long tmp = pawns; tmp; tmp &= tmp - 1) {
        int sq = __builtin_ffsll(tmp) - 1;
        int f = Lib::getFile(sq);
        unsigned long long leftMask = 0ULL, rightMask = 0ULL;
        if (f > 0)   for (int r = 0; r < 8; ++r) leftMask  |= (1ULL << (r*8 + (f-1)));
        if (f < 7)   for (int r = 0; r < 8; ++r) rightMask |= (1ULL << (r*8 + (f+1)));
        bool hasLeft  = ((pawns & leftMask)  != 0);
        bool hasRight = ((pawns & rightMask) != 0);
        if (!(hasLeft || hasRight)) pen += ISOLATED_PAWN_PEN;
    }
    return pen;
}

int passed_pawn_bonus(const Board& b, bool white) {
    unsigned long long myPawns = (b.pawns & (white ? b.whites : b.blacks));
    unsigned long long oppPawns= (b.pawns & (white ? b.blacks : b.whites));
    int bonus = 0;
    for (unsigned long long tmp = myPawns; tmp; tmp &= tmp - 1) {
        int sq = __builtin_ffsll(tmp) - 1;
        int f = Lib::getFile(sq);
        int r = Lib::getRank(sq);
        unsigned long long mask = 0ULL;
        if (white) {
            for (int rr = r+1; rr < 8; ++rr) {
                for (int df = -1; df <= 1; ++df) {
                    int ff = f + df;
                    if (ff >= 0 && ff < 8) mask |= (1ULL << (rr*8 + ff));
                }
            }
        } else {
            for (int rr = r-1; rr >= 0; --rr) {
                for (int df = -1; df <= 1; ++df) {
                    int ff = f + df;
                    if (ff >= 0 && ff < 8) mask |= (1ULL << (rr*8 + ff));
                }
            }
        }
        if ((oppPawns & mask) == 0) {
            int rankFromWhite = white ? (r+1) : (8-r);
            int idx = std::max(0, std::min(7, rankFromWhite));
            bonus += PASSED_PAWN_BONUS[idx];
        }
    }
    return bonus;
}

void eval_side_PST(const Board& b, bool white, int& mg, int& eg) {
    unsigned long long side = white ? b.whites : b.blacks;

    auto add_piece = [&](unsigned long long bb, const std::array<int,64>& mgpst, const std::array<int,64>& egpst, int valMG, int valEG) {
        unsigned long long bbSide = bb & side;
        for (unsigned long long tmp = bbSide; tmp; tmp &= tmp - 1) {
            int sq = __builtin_ffsll(tmp) - 1;
            int idx = white ? sq : mirror_rank(sq);
            mg += valMG + mgpst[idx];
            eg += valEG + egpst[idx];
        }
    };

    add_piece(b.pawns,   PAWN_PST_MG,   PAWN_PST_EG,   PAWN_MG,   PAWN_EG);
    add_piece(b.knights, KNIGHT_PST_MG, KNIGHT_PST_EG, KNIGHT_MG, KNIGHT_EG);
    add_piece(b.bishops, BISHOP_PST_MG, BISHOP_PST_EG, BISHOP_MG, BISHOP_EG);
    add_piece(b.rooks,   ROOK_PST_MG,   ROOK_PST_EG,   ROOK_MG,   ROOK_EG);
    add_piece(b.queens,  QUEEN_PST_MG,  QUEEN_PST_EG,  QUEEN_MG,  QUEEN_EG);

    unsigned long long kingSide = b.kings & side;
    if (kingSide) {
        int sq = __builtin_ffsll(kingSide) - 1;
        int idx = white ? sq : mirror_rank(sq);
        mg += KING_PST_MG[idx];
        eg += KING_PST_EG[idx];
    }
}

} // namespace

int Evaluation::evaluate(Board& board)
{
    int phase = game_phase(board);

    int wmg=0,wep=0, bmg=0,bep=0;
    eval_side_PST(board, true,  wmg, wep);
    eval_side_PST(board, false, bmg, bep);

    if (__builtin_popcountll(board.bishops & board.whites) >= 2) { wmg += BISHOP_PAIR_BONUS; wep += BISHOP_PAIR_BONUS; }
    if (__builtin_popcountll(board.bishops & board.blacks) >= 2) { bmg += BISHOP_PAIR_BONUS; bep += BISHOP_PAIR_BONUS; }

    auto rook_bonus = [&](bool white){
        int bonusMG = 0, bonusEG = 0;
        auto rooks = board.rooks & (white ? board.whites : board.blacks);
        for (unsigned long long tmp = rooks; tmp; tmp &= tmp - 1) {
            int sq = __builtin_ffsll(tmp) - 1;
            int file = Lib::getFile(sq);
            unsigned long long fileMask = 0ULL;
            for (int r = 0; r < 8; ++r) fileMask |= (1ULL << (r*8 + file));
            auto pawnsFile = board.pawns & fileMask;
            bool open = (pawnsFile == 0);
            bool semi = false;
            if (!open) {
                auto wp = (board.pawns & board.whites) & fileMask;
                auto bp = (board.pawns & board.blacks) & fileMask;
                if (white) semi = (wp == 0) && (bp != 0);
                else       semi = (bp == 0) && (wp != 0);
            }
            if (open)      { bonusMG += ROOK_OPEN_FILE; bonusEG += ROOK_OPEN_FILE; }
            else if (semi) { bonusMG += ROOK_SEMI_OPEN; bonusEG += ROOK_SEMI_OPEN; }
        }
        return std::pair<int,int>(bonusMG, bonusEG);
    };
    auto rbw = rook_bonus(true);  wmg += rbw.first; wep += rbw.second;
    auto rbb = rook_bonus(false); bmg += rbb.first; bep += rbb.second;

    int isoW = isolated_pawn_penalty(board, true);
    int isoB = isolated_pawn_penalty(board, false);
    int dblW = doubled_pawn_penalty(board, true);
    int dblB = doubled_pawn_penalty(board, false);
    int passW = passed_pawn_bonus(board, true);
    int passB = passed_pawn_bonus(board, false);

    wmg += -isoW - dblW + passW; wep += -isoW - dblW + passW;
    bmg += -isoB - dblB + passB; bep += -isoB - dblB + passB;

    int white = (wmg * phase + wep * (24 - phase)) / 24;
    int black = (bmg * phase + bep * (24 - phase)) / 24;
    int score = white - black;

    score = std::max(-SCORE_CLAMP, std::min(SCORE_CLAMP, score));
    return score;
}
