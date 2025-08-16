#include "Uci.h"

namespace {
const std::string START_POS = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
}

Uci::Uci(Comm *comm)
{
	this->comm = comm;

	this->comm->registerUciOutputCallback( [this](std::string message) { this->sendString(message); } );
}

void Uci::run()
{
	std::string input = "";
	
	while(true)
	{
		std::getline(std::cin,input);
		this->parse(input);
	}
}

void Uci::parse(std::string p_parameters)
{
	std::vector<std::string> parameters = Lib::split(p_parameters, ' ');
	if (parameters.size() == 0)
	{
	        return;
	}
	
        // parse command
        std::string command = parameters[0];
        static const std::string prefix = "info string [Uci::parse]";
	
	if (command.compare("uci") == 0)
	{
		this->sendId("name", "CHESSna 2 Version 1.10 - 202508162238");
		this->sendId("author", "Peter Schneider");
		this->sendUciok();
	}
	else if (command.compare("isready") == 0)
	{
		this->sendReadyok();
	}
        else if (command.compare("go") == 0)
        {
                GoParams params;
                for (size_t i = 1; i + 1 < parameters.size(); ++i)
                {
                        try
                        {
                                int value = std::stoi(parameters[i + 1]);
                                if (parameters[i] == "movetime")
                                {
                                        params.movetime = value;
                                }
                                else if (parameters[i] == "wtime")
                                {
                                        params.wtime = value;
                                }
                                else if (parameters[i] == "btime")
                                {
                                        params.btime = value;
                                }
                                else if (parameters[i] == "winc")
                                {
                                        params.winc = value;
                                }
                                else if (parameters[i] == "binc")
                                {
                                        params.binc = value;
                                }
                        }
                        catch(const std::exception &)
                        {
                                // ignore invalid value
                        }
                }
                this->comm->engineGo(params);
        }
        else if (command.compare("quit") == 0 || command.compare("exit") == 0)
        {
                //this->comm->engineStop();
                std::exit(0);
        }
        else if (command.compare("stop") == 0)
        {
                this->comm->engineStop();
        }
	else if (command.compare("position") == 0)
	{
	        if (parameters.size() < 2)
	        {
                        this->sendString(prefix + " Error: position requires arguments\n");
	                return;
	        }

	        std::vector<std::string>::iterator tokenIterator = parameters.begin();
	        ++tokenIterator; // skip command
                std::string token = *tokenIterator;

                std::string fen = START_POS;
	        if (token.compare("startpos") == 0)
	        {
	                ++tokenIterator;
	        }
	        else if (token.compare("fen") == 0)
	        {
	                if (parameters.size() < 8)
	                {
                                this->sendString(prefix + " Error: position fen requires 6 fields\n");
	                        return;
	                }
	                ++tokenIterator; // skip 'fen'
	                std::ostringstream f;
	                for (int i = 0; i < 6; ++i, ++tokenIterator)
	                {
	                        f << *tokenIterator;
	                        if (i < 5) f << " ";
	                }
	                fen = f.str();
	        }
	        else
	        {
                        this->sendString(prefix + " Error: position requires 'startpos' or 'fen'\n");
	                return;
	        }

	        this->comm->engineSetPosition(fen);

	        if (tokenIterator != parameters.end() && (*tokenIterator).compare("moves") == 0)
	        {
	                ++tokenIterator; // skip 'moves'
	                for ( ; tokenIterator != parameters.end(); ++tokenIterator)
	                {
	                        this->comm->engineExecuteMove(*tokenIterator);
	                }
	        }
	}
	
	// custom commands
        else if (command.compare("help") == 0)
        {
                std::ostringstream help;
                help << prefix << " CHESSna help for debugging console" << std::endl;
                help << prefix << std::endl;
                help << prefix << "  board  Displays debug output." << std::endl;
                help << prefix << "  moves  Lists all legal moves for the current position." << std::endl;
                help << prefix << "  perft <depth>  Performance test of the move generation." << std::endl;
                help << prefix << "  perftdiv <depth>  Performance test of the move generation with debug output." << std::endl;
                help << prefix << "  evaluate  Evaluate the current position." << std::endl;
                help << prefix << "  <move>  Enter a move like e2e4 to play it." << std::endl;
                this->sendString(help.str());
        }
        else if (command.compare("board") == 0)
        {
                this->comm->engineDebug();
        }
        else if (command.compare("moves") == 0)
        {
                this->comm->engineListMoves();
        }
        else if (command.compare("perft") == 0)
        {
                if (parameters.size() < 2 || parameters[1].empty())
                {
                        for (int d = 1; d <= 5; ++d)
                        {
                                this->comm->enginePerftNodes(d);
                        }
                        return;
                }
                try
                {
                        int depth = std::stoi(parameters[1]);
                        if (depth < 0)
                        {
                                this->sendString(prefix + " Error: perft depth must be non-negative\n");
                                return;
                        }
                        this->comm->enginePerft(depth);
                }
                catch(const std::exception &)
                {
                        this->sendString(prefix + " Error: perft requires numeric depth\n");
                }
	}
        else if (command.compare("perftdiv") == 0)
        {
                if (parameters.size() < 2 || parameters[1].empty())
                {
                        this->sendString(prefix + " Error: perftdiv requires depth parameter\n");
                        return;
                }
                try
                {
                        int depth = std::stoi(parameters[1]);
                        if (depth < 0)
                        {
                                this->sendString(prefix + " Error: perftdiv depth must be non-negative\n");
                                return;
                        }
                        this->comm->enginePerftDivide(depth);
                }
                catch(const std::exception &)
                {
                        this->sendString(prefix + " Error: perftdiv requires numeric depth\n");
                }
        }
        else if (command.compare("evaluate") == 0)
        {
                this->comm->engineEvaluate();
        }
        else if ((command.length() == 4 || command.length() == 5) &&
                 command[0] >= 'a' && command[0] <= 'h' &&
                 command[1] >= '1' && command[1] <= '8' &&
                 command[2] >= 'a' && command[2] <= 'h' &&
                 command[3] >= '1' && command[3] <= '8' &&
                 (command.length() == 4 || command[4] == 'q' || command[4] == 'r' || command[4] == 'b' || command[4] == 'n'))
        {
                this->comm->engineExecuteMove(command);
        }
        else
        {
                // ignore unknown command
        }
}


void Uci::sendString(std::string message)
{
        // When the engine communicates with another process (e.g. lichess-bot),
        // std::cout is fully buffered. Without an explicit flush the final
        // "bestmove" line might remain in the buffer, causing the bot to think
        // the engine timed out. Flush after every message to ensure it is
        // delivered immediately.
        std::cout << message << std::flush;
}

void Uci::sendId(std::string key, std::string value)
{
	std::cout << "id " << key << " " << value << std::endl;
}

void Uci::sendUciok()
{
	std::cout << "uciok" << std::endl;
}

void Uci::sendReadyok()
{
	std::cout << "readyok" << std::endl;
}

void Uci::sendBestmove(std::string move, std::string ponder)
{
	std::cout << "bestmove " << move;
	if (ponder.compare("") != 0)
	{
		std::cout << " ponder " << ponder;
	}
	std::cout << std::endl;
}
