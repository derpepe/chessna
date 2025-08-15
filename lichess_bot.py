#!/opt/homebrew/bin/python3
import subprocess
import threading
import queue
import time
import berserk

# --- Configuration ---
# TODO: The user should replace this with their own Lichess API token.
# You can create a token at: https://lichess.org/account/oauth/token/create
LICHESS_API_TOKEN = "YOUR_LICHESS_API_TOKEN"

# Path to the chess engine executable.
# On Windows, this is likely 'bin/chessna.exe'.
# On Linux/macOS, you might need to compile it first (`make`) and use 'bin/chessna'.
ENGINE_PATH = "bin/chessna.exe"

# Time to think per move, in milliseconds.
# The `stop` command is buggy, so we use a fixed time.
THINK_TIME_MS = 1000

class Engine:
    """A class to manage the UCI chess engine subprocess."""

    def __init__(self, engine_path):
        self.engine_path = engine_path
        self.process = None
        self.output_queue = queue.Queue()
        self.is_running = False

    def start(self):
        """Starts the engine process and the output reading thread."""
        try:
            self.process = subprocess.Popen(
                self.engine_path,
                stdin=subprocess.PIPE,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                universal_newlines=True,
                bufsize=1,  # Line-buffered
            )
            self.is_running = True

            # Start a thread to read the engine's output
            self.output_thread = threading.Thread(target=self._read_output)
            self.output_thread.daemon = True
            self.output_thread.start()

            print("Engine process started.")
            return True
        except FileNotFoundError:
            print(f"Error: Engine executable not found at '{self.engine_path}'")
            print("Please check the ENGINE_PATH configuration.")
            return False

    def _read_output(self):
        """Reads lines from the engine's stdout and puts them in a queue."""
        while self.is_running:
            try:
                line = self.process.stdout.readline()
                if line:
                    self.output_queue.put(line.strip())
                else:
                    # Process exited
                    break
            except Exception as e:
                print(f"Error reading engine output: {e}")
                break
        print("Engine output reader thread stopped.")

    def send_command(self, command):
        """Sends a command to the engine."""
        if self.process and self.process.stdin:
            print(f"GUI -> Engine: {command}")
            self.process.stdin.write(command + "\n")
            self.process.stdin.flush()

    def get_response(self, expected_prefix, timeout=10):
        """Waits for a specific response from the engine."""
        start_time = time.time()
        while time.time() - start_time < timeout:
            try:
                line = self.output_queue.get(timeout=0.1)
                print(f"Engine -> GUI: {line}")
                if line.startswith(expected_prefix):
                    return line
            except queue.Empty:
                continue
        return None

    def stop(self):
        """Stops the engine process."""
        if self.process:
            self.is_running = False
            self.send_command("quit")
            try:
                self.process.wait(timeout=2)
            except subprocess.TimeoutExpired:
                self.process.kill()
            print("Engine process stopped.")

    def uci_handshake(self):
        """Performs the initial UCI handshake."""
        self.send_command("uci")
        if not self.get_response("uciok"):
            print("Error: Engine did not respond with 'uciok'.")
            return False

        self.send_command("isready")
        if not self.get_response("readyok"):
            print("Error: Engine did not respond with 'readyok'.")
            return False

        return True

    def get_best_move(self, moves_string, think_time_ms):
        """Gets the best move for a given position."""
        position_cmd = f"position startpos moves {moves_string}" if moves_string else "position startpos"
        self.send_command(position_cmd)
        self.send_command(f"go movetime {think_time_ms}")

        response = self.get_response("bestmove", timeout=(think_time_ms / 1000) + 5)
        if response and response.startswith("bestmove"):
            return response.split(" ")[1]
        return None


class LichessBot:
    """The main class for the Lichess bot."""

    def __init__(self, token, engine_path, think_time):
        self.token = token
        self.engine_path = engine_path
        self.think_time = think_time
        self.client = None
        self.engine = None

    def run(self):
        """Starts the bot and listens for events."""
        if self.token == "YOUR_LICHESS_API_TOKEN":
            print("Please configure your LICHESS_API_TOKEN in the script.")
            return

        # Start the engine
        self.engine = Engine(self.engine_path)
        if not self.engine.start():
            return

        if not self.engine.uci_handshake():
            self.engine.stop()
            return

        print("Engine initialized. Connecting to Lichess...")

        # Connect to Lichess
        session = berserk.TokenSession(self.token)
        self.client = berserk.Client(session)

        # Start listening for events
        self.listen_for_events()

    def listen_for_events(self):
        print("Listening for Lichess events (challenges, game starts)...")
        for event in self.client.bots.stream_incoming_events():
            if event['type'] == 'challenge':
                self.handle_challenge(event['challenge'])
            elif event['type'] == 'gameStart':
                # Start a new thread to handle the game
                game_thread = threading.Thread(target=self.play_game, args=(event['game']['id'],))
                game_thread.daemon = True
                game_thread.start()

    def handle_challenge(self, challenge):
        """Handles incoming challenges."""
        # For now, accept any challenge from anyone.
        # You might want to add filters here (e.g., time control, variant).
        try:
            print(f"Accepting challenge from {challenge['challenger']['id']}")
            self.client.bots.accept_challenge(challenge['id'])
        except berserk.exceptions.ResponseError as e:
            print(f"Error accepting challenge: {e}")

    def play_game(self, game_id):
        """Manages a single game."""
        print(f"Starting to play game: {game_id}")
        game_stream = self.client.bots.stream_game_state(game_id)

        # The first event is the full game state
        initial_state = next(game_stream)

        my_color = "white" if initial_state['white']['id'].lower() == self.client.account.get()['id'].lower() else "black"
        print(f"I am playing as {my_color}.")

        # Handle the full game state first
        self.handle_game_state(game_id, initial_state, my_color)

        # Then handle subsequent updates
        for game_event in game_stream:
            if game_event['type'] == 'gameState':
                self.handle_game_state(game_id, game_event, my_color)
            elif game_event['type'] == 'chatLine':
                # Can add chat logic here
                pass

    def handle_game_state(self, game_id, game_state, my_color):
        """Analyzes the game state and makes a move if it's our turn."""
        # Check if game is over
        if game_state['status'] not in ['created', 'started']:
            print(f"Game {game_id} is over. Status: {game_state['status']}")
            return

        # Determine whose turn it is
        turn = len(game_state['moves'].split()) % 2
        is_my_turn = (my_color == 'white' and turn == 0) or \
                      (my_color == 'black' and turn == 1)

        if not is_my_turn:
            return

        print(f"It's my turn in game {game_id}.")

        # Get the best move from the engine
        moves_string = game_state['moves']
        best_move = self.engine.get_best_move(moves_string, self.think_time)

        if best_move:
            try:
                print(f"Making move '{best_move}' in game {game_id}")
                self.client.bots.make_move(game_id, best_move)
            except berserk.exceptions.ResponseError as e:
                # This can happen if the move is illegal or the game ended
                print(f"Error making move in game {game_id}: {e}")
        else:
            print(f"Engine did not provide a move for game {game_id}.")


if __name__ == "__main__":
    print("Starting Lichess Bot...")
    bot = LichessBot(LICHESS_API_TOKEN, ENGINE_PATH, THINK_TIME_MS)
    try:
        bot.run()
    except KeyboardInterrupt:
        print("\nBot shutting down.")
    finally:
        if bot.engine:
            bot.engine.stop()
