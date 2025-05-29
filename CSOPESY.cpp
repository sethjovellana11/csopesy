#include <iostream>
#include <string>

void displayHeader();

int main()
{
	// initialize, screen, scheduler-test, scheduler-stop, report-util, clear, and exit
    std::string cmd;
    displayHeader();

    // command loop
    while (cmd != "exit") {
        // get input
		std::cout << ">";
        std::getline(std::cin, cmd);
        if (cmd == "clear") {
            // system("cls") for windowsOS
            system("clear");
            displayHeader();
        }
        else if (cmd == "initialize") {
            std::cout << "'initialize' command recognized. Doing something.\n";
        }
        else if (cmd == "screen") {
            std::cout << "'screen' command recognized. Doing something.\n";
        }
        else if (cmd == "scheduler-test") {
            std::cout << "'scheduler-test' command recognized. Doing something.\n";
        }
        else if (cmd == "scheduler-stop") {
            std::cout << "'scheduler-stop' command recognized. Doing something.\n";
        }
        else if (cmd == "report-util") {
            std::cout << "'report-util' command recognized. Doing something.\n";
        }
        else if (cmd == "exit") {
			std::cout << "Goodbye!\n";
        }
        else {
            std::cout << "'" << cmd << "' command not recognized. Please try again.\n";
        }
    }
    return 0;
}

void displayHeader()
{
//        ___           ___           ___                         ___           ___                   
//       /  /\         /  /\         /  /\          ___          /  /\         /  /\          __      
//      /  /::\       /  /::\       /  /::\        /  /\        /  /::\       /  /::\        |  |\    
//     /  /:/\:\     /__/:/\:\     /  /:/\:\      /  /::\      /  /:/\:\     /__/:/\:\       |  |:|   
//    /  /:/  \:\   _\_ \:\ \:\   /  /:/  \:\    /  /:/\:\    /  /::\ \:\   _\_ \:\ \:\      |  |:|   
//   /__/:/ \  \:\ /__/\ \:\ \:\ /__/:/ \__\:\  /  /::\ \:\  /__/:/\:\ \:\ /__/\ \:\ \:\     |__|:|__ 
//   \  \:\  \__\/ \  \:\ \:\_\/ \  \:\ /  /:/ /__/:/\:\_\:\ \  \:\ \:\_\/ \  \:\ \:\_\/     /  /::::\
//    \  \:\        \  \:\_\:\    \  \:\  /:/  \__\/  \:\/:/  \  \:\ \:\    \  \:\_\:\      /  /:/~~~~
//     \  \:\        \  \:\/:/     \  \:\/:/        \  \::/    \  \:\_\/     \  \:\/:/     /__/:/     
//      \  \:\        \  \::/       \  \::/          \__\/      \  \:\        \  \::/      \__\/      
//       \__\/         \__\/         \__\/                       \__\/         \__\/     

// Sourced from: https://patorjk.com/software/taag/, font: Isometric4

	std::cout << "\033[32m"; // making ascii green
    std::cout << "        ___           ___           ___                         ___           ___                   \n";
	std::cout << "       /  /\\         /  /\\         /  /\\          ___          /  /\\         /  /\\          __      \n";
	std::cout << "      /  /::\\       /  /::\\       /  /::\\        /  /\\        /  /::\\       /  /::\\        |  |\\    \n";
	std::cout << "     /  /:/\\:\\     /__/:/\\:\\     /  /:/\\:\\      /  /::\\      /  /:/\\:\\     /__/:/\\:\\       |  |:|   \n";
	std::cout << "    /  /:/  \\:\\   _\\_ \\:\\ \\:\\   /  /:/  \\:\\    /  /:/\\:\\    /  /::\\ \\:\\   _\\_ \\:\\ \\:\\      |  |:|   \n";
	std::cout << "   /__/:/ \\  \\:\\ /__/\\ \\:\\:\\:\\ /__/:/ \\__\\:\\  /  /::\\ \\:\\  /__/:/\\:\\ \\:\\ /__/\\ \\:\\:\\:\\     |__|:|__ \n";
	std::cout << "   \\  \\:\\  \\__\\/ \\  \\:\\ \\:\\_\\/ \\  \\:\\ /  /:/ /__/:/\\:\\_\\:\\ \\  \\:\\ \\:\\_\\/ \\  \\:\\ \\:\\_\\/     /  /::::\\\n";
	std::cout << "    \\  \\:\\        \\  \\:\\_\\:\\    \\  \\:\\  /:/  \\__\\\/  \\:\\/:/  \\  \\:\\ \\:\\    \\  \\:\\_\\:\\      /  /:/~~~~\n";
	std::cout << "     \\  \\:\\        \\  \\:\\/:/     \\  \\:\\/:/        \\  \\::/    \\  \\:\\_\\/     \\  \\:\\/:/     /__/:/     \n";
	std::cout << "      \\  \\:\\        \\  \\::/       \\  \\::/          \\__\\/      \\  \\:\\        \\  \\::/      \\__\\/      \n";
	std::cout << "       \\__\\/         \\__\\/         \\__\\/                       \\__\\/         \\__\\/                   \n";

	// change x in \033[xm to change color idk what looks nice haha (30-37 for foreground, 40-47 for background)
    std::cout << "\033[33mHello, welcome to CSOPESY commandline!\033[0m\n";
    std::cout << "\033[36mType 'exit' to quit, 'clear' to clear the screen.\033[0m\n";
}
