 #include "Evaluation.h"
 #include "Board.h"

 int Evaluation::evaluate(Board& board)
 {
         int score = 0;

         // 0. Evaluate material on the board
         int whiteMaterial =
                 100   * __builtin_popcountll(board.pawns   & board.whites) +
                 320   * __builtin_popcountll(board.knights & board.whites) +
                 330   * __builtin_popcountll(board.bishops & board.whites) +
                 500   * __builtin_popcountll(board.rooks   & board.whites) +
                 900   * __builtin_popcountll(board.queens  & board.whites) +
                 20000 * __builtin_popcountll(board.kings   & board.whites);

         int blackMaterial =
                 100   * __builtin_popcountll(board.pawns   & board.blacks) +
                 320   * __builtin_popcountll(board.knights & board.blacks) +
                 330   * __builtin_popcountll(board.bishops & board.blacks) +
                 500   * __builtin_popcountll(board.rooks   & board.blacks) +
                 900   * __builtin_popcountll(board.queens  & board.blacks) +
                 20000 * __builtin_popcountll(board.kings   & board.blacks);

         score += whiteMaterial - blackMaterial;

         // Reward central pawns
         unsigned long long centralMask = (1ULL << 27) | (1ULL << 28) | (1ULL << 35) | (1ULL << 36);
         int whiteCentral = __builtin_popcountll(board.pawns & board.whites & centralMask);
         int blackCentral = __builtin_popcountll(board.pawns & board.blacks & centralMask);
         score += 22 * whiteCentral;
         score -= 22 * blackCentral;

         // Penalize knights and bishops on their starting squares
         unsigned long long whiteStartMask = (1ULL << 1) | (1ULL << 2) | (1ULL << 5) | (1ULL << 6);
         unsigned long long blackStartMask = (1ULL << 57) | (1ULL << 58) | (1ULL << 61) | (1ULL << 62);
         int whiteStartPieces = __builtin_popcountll((board.knights | board.bishops) & board.whites & whiteStartMask);
         int blackStartPieces = __builtin_popcountll((board.knights | board.bishops) & board.blacks & blackStartMask);
         score -= 5 * whiteStartPieces;
         score += 5 * blackStartPieces;

        // Penalize knights on the rim
        unsigned long long edgeMask = 0xff818181818181ffULL;
        int whiteEdgeKnights = __builtin_popcountll(board.knights & board.whites & edgeMask);
        int blackEdgeKnights = __builtin_popcountll(board.knights & board.blacks & edgeMask);
        score -= 5 * whiteEdgeKnights;
        score += 5 * blackEdgeKnights;
        
        // Reward ability to castle
        if (board.casteling_K || board.casteling_Q)
                score += 20;
        if (board.casteling_k || board.casteling_q)
                score -= 20;
        
        return score;
}
