const { Chess } = require('chess.js');

function perft(chess, depth) {
  if (depth === 0) {
    return 1;
  }

  let nodes = 0;
  const moves = chess.moves();

  for (let i = 0; i < moves.length; i++) {
    chess.move(moves[i]);
    nodes += perft(chess, depth - 1);
    chess.undo();
  }

  return nodes;
}

function divide(fen, depth) {
  const chess = new Chess(fen);
  const moves = chess.moves();
  let totalNodes = 0;

  console.log('Divide for FEN: ' + fen + ' at depth ' + depth);
  console.log('---------------------------------');

  for (let i = 0; i < moves.length; i++) {
    const move = moves[i];
    chess.move(move);
    const nodes = perft(chess, depth - 1);
    totalNodes += nodes;
    // The move object from chess.js is more detailed, so we use the `san` property for the output
    const moveObject = chess.history({ verbose: true }).slice(-1)[0];
    console.log(moveObject.san + ': ' + nodes);
    chess.undo();
  }

  console.log('---------------------------------');
  console.log('Total nodes: ' + totalNodes);
}

const fen = "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1";
divide(fen, 2);
