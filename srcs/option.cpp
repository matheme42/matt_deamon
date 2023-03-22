#include "option.hpp"


Option::Option() {
    working_option = '\0';
    port = 4242;
}


int Option::checkWorkingArg(char *s) {
    int i = 0;

    if (state[working_option] == type_t::number) {
        while (s[i] && isdigit(s[i++])) ;
        if (s[i]) {
            std::cout << "option -" << working_option << ": " << ORANGE << "must be an int" << DEFAULT_COLOR << std::endl;
            exit(EXIT_FAILURE);
        }
        return (i);
    }
    if (state[working_option] == type_t::sentence) {
        if (strlen(s) == 0) {
            std::cout << "option -" << working_option << ": " << ORANGE << "can't be empty" << DEFAULT_COLOR << std::endl;
            exit(EXIT_FAILURE);
        }
        while (s[i] && isprint(s[i++])) ;
        if (s[i]) {
            std::cout << "option -" << working_option << ": " << ORANGE << "must be a printable string" << DEFAULT_COLOR << std::endl;
            exit(EXIT_FAILURE);
        }
        return (i);
    }
    if (state[working_option] == type_t::infile) {
        if (FILE *file = fopen(s, "r")) {
            fclose(file);
        } else {
            if (!strcmp(s, "\0")) {
                std::cout << "option -" << working_option << ": " << ORANGE << "a file must be reffered" << DEFAULT_COLOR << std::endl;
            } else {
                std::cout << "option -" << working_option << ": " << ORANGE << s << " does not exist" << DEFAULT_COLOR << std::endl;
            }
            exit(EXIT_FAILURE); 
        }
        return (strlen(s));
    }
    if (state[working_option] == type_t::outfile) {
        if (FILE *file = fopen(s, "r+")) {
            fclose(file);
            std::string line;
            std::cout << "option -" << working_option << ": " << DARK_BLUE << "a file already exist override / append / cancel ? (O / A / C): " << DEFAULT_COLOR;
            std::getline(std::cin, line);
            if (line[0] == 'C' || line[0] == 'c') exit(0);
            else if (line[0] == 'O' || line[0] == 'o') remove(s);
        } else if (FILE *file = fopen(s, "w")) {
            fclose(file);
            remove(s);
        } else {
            if (!strcmp(s, "\0")) {
                std::cout << "option -" << working_option << ":" << ORANGE <<  "a file path must be reffered" << DEFAULT_COLOR << std::endl;
            } else {
                std::cout << "option -" << working_option << ": " << ORANGE << s << " is not a valid file path" << DEFAULT_COLOR << std::endl;
            }
            exit(EXIT_FAILURE);
        }


        return (strlen(s));
    }
    return i;
}


void Option::usage() {
    std::cout << DARK_BLUE << "Usage:" << DEFAULT_COLOR << std::endl;
    #ifdef MATTDEAMONCLIENT
    std::cout <<  GREY << "    ./matt_daemon " << ORANGE << "[host]" << DARK_ORANGE << " [-k decryptage key] [-P master password]" << LIGHT_GREY << " -p port "  << std::endl;
    #else
    std::cout << GREY << "    ./matt_daemon " << DARK_ORANGE << "[-k cryptage key] [-P master password] " << LIGHT_GREY << "-p port" << std::endl;
    #endif
    std::cout << WHITE_BOLD << "  options:" << DEFAULT_COLOR << std::endl;
    std::cout << GREY << "    -p: " << LIGHT_GREY << "port number between 0 and 65535" << DEFAULT_COLOR << std::endl;
    std::cout << GREY << "    -P: " << LIGHT_GREY << "set a master password to the server" << DEFAULT_COLOR << std::endl;
    std::cout << GREY << "    -k: " << LIGHT_GREY << "set a cryptage key use to encrypt the connection" << DEFAULT_COLOR << std::endl;

    exit(0);
}

std::string Option::resolve_dns(const std::string& domain_name) {
    addrinfo hints{};
    hints.ai_family = AF_INET; // Recherche IPv4 uniquement
    hints.ai_socktype = SOCK_STREAM; // Type de socket : TCP

    addrinfo* result;
    if (getaddrinfo(domain_name.c_str(), nullptr, &hints, &result) != 0) {
        return "";
    }

    char ip_str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &reinterpret_cast<sockaddr_in*>(result->ai_addr)->sin_addr, ip_str, INET_ADDRSTRLEN);

    freeaddrinfo(result);

    return ip_str;
}

int Option::fillOption(char *s) {
    int i = 0;
    i = checkWorkingArg(s);
    if (working_option == 'h') usage();
    else if (working_option == 'k') crytage_key = s;
    else if (working_option == 'p') {
        port = atoi(s);
        if (port <= 0 || port > 65535) {
            std::cout << "option -" << working_option << ": " << ORANGE << "must between 0 and 65535" << DEFAULT_COLOR << std::endl;
            exit(EXIT_FAILURE); 
        }
    } else if (working_option == 'P') {
        master_password = s;
    } 
    else if (working_option == '@') {
        ip = resolve_dns(s);
        if (ip == "") {
            std::cout << ORANGE << "unable to resolve the IP for hostname: " << DEFAULT_COLOR << s << std::endl;
            exit(EXIT_FAILURE); 
        }
    }

    return (i);
}

/*
** permet de scruter les options
** aussi bien sur le meme arg que sur plusieur arg
*/

void Option::parse(int ac, char **av)
{
	int	i;

    if ((*av)[0] == '-' && (*av)[1] == '@'){
        std::cout << DARK_BLUE "matt_daemon: " << DEFAULT_COLOR << (*av)[1]  << ORANGE << " is not an option" << DEFAULT_COLOR << std::endl;
        exit(EXIT_FAILURE);
    }

	if (((*av)[0] != '-' || !(*av)[1]) && working_option == '\0') {
		working_option = (char)'@';
    }
    i = working_option == '\0' ? 1 : 0;
	while ((*av)[i])
	{
        if (working_option != '\0') {
            i += fillOption(&((*av)[i]));
            working_option = '\0';
            continue;
        }
        working_option = '\0';
        if (state.find((*av)[i]) == state.end()) {
            std::cout  << DARK_BLUE << "matt_daemon: " << DEFAULT_COLOR << (*av)[i] << ORANGE  << " is not an option" << DEFAULT_COLOR << std::endl;
            exit(EXIT_FAILURE);
        }
        working_option = (*av)[i];
        i++;
	}
	if (ac > 1) {
		parse(ac - 1, &av[1]);
        return ;
    }
    if (working_option != '\0') {
        fillOption((char*)"\0");
    }
}

/*
** permet de ce diriger apres les options
** la fonction prend en compte le cas ou il n'y a que le tiret
** et aucune option
** dans ce cas la fonction ne deplace pas le pointeur
*/

char	**Option::ft_go_after_option(char **av)
{
	if (*av && **av == '-' && (*av)[1])
		return (ft_go_after_option(&av[1]));
	return (av);
}

/*
** fonction visible de l'exterieur
** elle permet de recuperer les options via ...
** ...l'adresse d'un int que l'on passe en parametre
** et elle retourne l'argument qui suit les options
*/

char	**Option::configure(int ac, char **av)
{
    if (ac == 1) return (&(av[1]));
	parse(ac - 1, &(av[1]));
	return (ft_go_after_option(&(av[1])));
}