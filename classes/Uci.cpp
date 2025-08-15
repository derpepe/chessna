#include "Uci.h"


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
	
	if (command.compare("uci") == 0)
	{
		this->sendId("name", "CHESSna 2 Version 0.01 alpha");
		this->sendId("author", "Peter Schneider");
		this->sendUciok();
	}
	else if (command.compare("isready") == 0)
	{
		this->sendReadyok();
	}
	else if (command.compare("go") == 0)
	{
		this->comm->engineGo();
	}
        else if (command.compare("quit") == 0 || command.compare("exit") == 0)
        {
                //this->comm->engineStop();
                std::exit(0);
        }
	else if (command.compare("stop") == 0)
	{
		this->sendBestmove("e2e4", ""); // TODO: get best move from engine
	}
	else if (command.compare("position") == 0)
	{
	        if (parameters.size() < 2)
	        {
	                this->sendString("info string Error: position requires arguments\n");
	                return;
	        }

	        std::vector<std::string>::iterator tokenIterator = parameters.begin();
	        ++tokenIterator; // skip command
	        std::string token = *tokenIterator;

	        std::string fen = Fen::startPos;
	        if (token.compare("startpos") == 0)
	        {
	                ++tokenIterator;
	        }
	        else if (token.compare("fen") == 0)
	        {
	                if (parameters.size() < 8)
	                {
	                        this->sendString("info string Error: position fen requires 6 fields\n");
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
	                this->sendString("info string Error: position requires 'startpos' or 'fen'\n");
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
                const std::string prefix = "info string [Uci::parse]";
                help << prefix << " CHESSna help for debugging console" << std::endl;
                help << prefix << std::endl;
                help << prefix << "  board  Displays debug output." << std::endl;
                help << prefix << "  perft <depth>  Performance test of the move generation." << std::endl;
                help << prefix << "  perftdiv <depth>  Performance test of the move generation with debug output." << std::endl;
                this->sendString(help.str());
        }
	else if (command.compare("board") == 0)
	{
		this->comm->engineDebug();
	}
	else if (command.compare("perft") == 0)
	{
	        if (parameters.size() < 2 || parameters[1].empty())
	        {
	                this->sendString("info string Error: perft requires depth parameter\n");
	                return;
	        }
	        try
	        {
	                int depth = std::stoi(parameters[1]);
	                if (depth < 0)
	                {
	                        this->sendString("info string Error: perft depth must be non-negative\n");
	                        return;
	                }
	                this->comm->enginePerft(depth);
	        }
	        catch(const std::exception &)
	        {
	                this->sendString("info string Error: perft requires numeric depth\n");
	        }
	}
        else if (command.compare("perftdiv") == 0)
        {
                if (parameters.size() < 2 || parameters[1].empty())
                {
                        this->sendString("info string Error: perftdiv requires depth parameter\n");
                        return;
                }
                try
                {
                        int depth = std::stoi(parameters[1]);
                        if (depth < 0)
                        {
                                this->sendString("info string Error: perftdiv depth must be non-negative\n");
                                return;
                        }
                        this->comm->enginePerftDivide(depth);
                }
                catch(const std::exception &)
                {
                        this->sendString("info string Error: perftdiv requires numeric depth\n");
                }
        }
	else
	{
		// ignore unknown command
	}
}


void Uci::sendString(std::string message)
{
	std::cout << message;
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
