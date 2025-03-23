// shutdown.cpp : Ce fichier contient la fonction 'main'. L'exécution du programme commence et se termine à cet endroit.
//

#include <Windows.h>
#include <iostream>
#include <time.h>
#include <synchapi.h>
#include <string>
#include <format>
#include <sstream>
#include <thread>
#include <filesystem>
#include <fstream>
#include <syncstream>
#include <mutex>

//namespace fs = std::filesystem;

struct Heure {
	int heure;
	int minute;
	int seconde;
};

typedef struct Heure Heure;

#define ETAT_EN_COURS "en_cours"
#define ETAT_FIN "fin"
#define AFFICHE(sstream) { m_screen.lock(); std::ostringstream str{ }; str << heureDebut() << sstream; std::cout << str.str()<<std::endl; m_screen.unlock(); }

std::mutex m_screen;
std::string etatCourant;

std::string heureDebut() {
	struct tm newtime;
	__time64_t long_time;
	errno_t err;
	std::string mystr;
	_time64(&long_time);
	// Convert to local time.
	err = _localtime64_s(&newtime, &long_time);
	if (err)
	{
		std::cerr << heureDebut() << "Invalid argument to _localtime64_s.\n";
		exit(1);
	}

	std::ostringstream oss;

	oss << newtime.tm_hour << ":" << newtime.tm_min << ":" << newtime.tm_sec << " : ";

	mystr = oss.str();
	//std::string formatted_str = std::format(
	//    "My name is {1:.2s} and pi is {0:.2f}", 1, 2);

	//return std::format("{}:{}:{}", newtime.tm_hour, newtime.tm_min, newtime.tm_sec);
	return mystr;
}

Heure* getHeure() {
	Heure* resultat;
	struct tm newtime;
	__time64_t long_time;
	errno_t err;

	// Get time as 64-bit integer.
	_time64(&long_time);
	// Convert to local time.
	err = _localtime64_s(&newtime, &long_time);
	if (err)
	{
		std::cerr << heureDebut() << "Invalid argument to _localtime64_s.\n";
		exit(1);
	}

	resultat = new Heure();
	resultat->heure = newtime.tm_hour;
	resultat->minute = newtime.tm_min;
	resultat->seconde = newtime.tm_sec;
	return resultat;
}

void affiche(Heure* heure) {
	if (heure != NULL) {
		std::cout << heureDebut() << "heure:" << heure->heure << ":" << heure->minute << ":" << heure->seconde << "\n";
	}
}

Heure* initialise(int argc, char* argv[]) {
	Heure* resultat;
	int heure, minute;
	bool trouve = false;
	heure = 15;
	minute = 0;

	AFFICHE("argc:"<< argc);

	if (argc >= 3) {
		char* heureStr = argv[1];
		char* minuteStr = argv[2];
		int heure2 = strtol(heureStr, NULL, 10);
		int minute2 = strtol(minuteStr, NULL, 10);
		if (heure2 > 0 && heure2 < 24 && minute2>0 && minute2 < 60) {
			trouve = true;
			heure = heure2;
			minute = minute2;
		}
	}
	resultat = new Heure();
	resultat->heure = heure;
	resultat->minute = minute;
	return resultat;
}



void arret() {

	//std::cout << heureDebut() << "Arret ...\n";
	AFFICHE("Arret ...");

	HANDLE hToken;
	TOKEN_PRIVILEGES tkp;

	// Get a token for this process. 

	if (!OpenProcessToken(GetCurrentProcess(),
		TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
		AFFICHE("Erreur: problème pour avoir le provilège d'arret du PC");
		return;
	}
		

	// Get the LUID for the shutdown privilege. 

	LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME,
		&tkp.Privileges[0].Luid);

	tkp.PrivilegeCount = 1;  // one privilege to set    
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	// Get the shutdown privilege for this process. 

	bool res2=AdjustTokenPrivileges(hToken, FALSE, &tkp, 0,
		(PTOKEN_PRIVILEGES)NULL, 0);

	int res3 = GetLastError();
	//if (res2!=0) {
	if(res3!=ERROR_SUCCESS) {
		AFFICHE("Erreur: problème pour ajuster le provilège d'arret du PC : "<<res2<<" ("<< res3 <<")");
		return;
	}

	//int res = ExitWindowsEx(EWX_POWEROFF, 0);
	int res = InitiateSystemShutdownEx(NULL,NULL,0,TRUE,FALSE, SHTDN_REASON_MAJOR_APPLICATION);
	if (res == 0) {
		AFFICHE("Arret OK");
		//std::cout << heureDebut() << "Arret OK\n";
	}
	else {
		AFFICHE("Arret erreur : "<<res);
		//std::cout << heureDebut() << "Arret erreur : " << res << "\n";
	}

}

bool limiteDepasse(Heure* heureCourante, Heure* heureLimite) {

	if (heureCourante == NULL) {
		std::cerr << heureDebut() << "heure courante null\n";
		exit(1);
	}
	if (heureLimite == NULL) {
		std::cerr << heureDebut() << "heure limite null\n";
		exit(1);
	}

	if (heureCourante->heure > heureLimite->heure) {
		return true;
	}
	else if (heureCourante->heure == heureLimite->heure) {
		if (heureCourante->minute >= heureLimite->minute) {
			return true;
		}
		else {
			return false;
		}
	}
	else {
		return false;
	}
}

long dureeMSecondes(Heure* heure) {
	if (heure == NULL) {
		std::cerr << heureDebut() << "heure courante null\n";
		exit(1);
	}
	long h = heure->heure * 60 * 60 + heure->minute * 60 + heure->seconde;
	return h * 1000;
}

long difference(Heure* heureCourante, Heure* heureLimite) {
	if (heureCourante == NULL) {
		std::cerr << heureDebut() << "heure courante null\n";
		exit(1);
	}
	if (heureLimite == NULL) {
		std::cerr << heureDebut() << "heure limite null\n";
		exit(1);
	}
	long h1 = 0, h2 = 0;
	h1 = dureeMSecondes(heureCourante);
	h2 = dureeMSecondes(heureLimite);
	if (h1 > h2) {
		return -1;
	}
	else {
		return h2 - h1;
	}
}

bool exists_test(const std::string& name) {
	struct stat buffer;
	return (stat(name.c_str(), &buffer) == 0);
}

std::string getEtat(std::string fichier) {
	std::ifstream infile(fichier);

	std::string line;
	std::string s = std::string();
	while (std::getline(infile, line))
	{
		AFFICHE("ligne: '" << line << "'");
		s = line;
		if (s.size() >= 4) {
			//AFFICHE("car1:" << (int)s.at(0) << ";" << (int)s.at(1) << ";" << (int)s.at(2)<< ";" << (int)s.at(3) << ".");
			//s = s.substr(2);
		}
		if (s.size() >= 8 && s.at(2) == 'f' && s.at(4) == 'i' && s.at(6) == 'n') {
			//AFFICHE("car1:" << (int)s.at(0) << ";" << (int)s.at(1) << ";" << (int)s.at(2) << ";" << (int)s.at(3) << ".");
			//s = s.substr(2);        
			return ETAT_FIN;
		}
		if (s.size() >= 8 && s.at(2) == 'e' && s.at(4) == 'n' && s.at(6) == '_' && s.at(8) == 'c' && s.at(10) == 'o' && s.at(12) == 'u' && s.at(14) == 'r' && s.at(16) == 's') {
			//AFFICHE("car1:" << (int)s.at(0) << ";" << (int)s.at(1) << ";" << (int)s.at(2) << ";" << (int)s.at(3) << ".");
			//s = s.substr(2);        
			return ETAT_EN_COURS;
		}
		break;
	}

	return s;
}

/*void affiche(std::ostringstream str) {
	std::cout << str.str();
}

void affiche2(const char* str) {
	m_screen.lock();
	printf("%s\n", str);
	m_screen.unlock();
}*/

// The function we want to execute on the new thread.
void task1(std::string fichier)
{

	/*m_screen.lock();
	std::cout << "task1 says: " << fichier;
	m_screen.unlock();*/
	//AFFICHE("task1");
	AFFICHE("task1 says: " << fichier);
	AFFICHE("Vérification de l'état du fichier: " << fichier);

	bool premier = true;
	while (true) {
		std::string etat;
		etat = getEtat(fichier);

		/*m_screen.lock();
		std::cout << heureDebut() << "etat: " << etat << "\n";
		std::cout << heureDebut() << "suite ...\n";
		m_screen.unlock();*/
		if (premier || etat != etatCourant) {
			AFFICHE("etat: '" << etat << "'");
			AFFICHE("suite ...");
			int n = etat.find(ETAT_FIN);
			AFFICHE("n=" << n);
			if (etat.find(ETAT_EN_COURS) != std::string::npos) {
				AFFICHE("en cours de backup...");
			}
			else if (etat.find(ETAT_FIN) != std::string::npos) {
				AFFICHE("backup fini");
				break;
			}
			else {
				AFFICHE("autre etat");
			}
			etatCourant = etat;
			premier = false;
		}
		Sleep(60 * 1000);
	}
	AFFICHE("fin de l'analyse de l'état");
}

void afficheDate() {
	time_t now = time(0);
	struct tm newtime;
	localtime_s(&newtime, &now);
	//tm* ltm = localtime(&now);

	std::string s;

	int annee = 1900 + newtime.tm_year;
	int mois = 1 + newtime.tm_mon;
	int jour = newtime.tm_mday;
	int heure = newtime.tm_hour;
	int minute = newtime.tm_min;
	int secondes = newtime.tm_sec;

	char buf[1024]; 
	sprintf_s(buf,sizeof(buf), "%02d/%02d/%04d %02d:%02d:%02d", jour,mois,annee,heure,minute,secondes);
	AFFICHE("date: "<< std::string(buf));
}

int main(int argc, char* argv[])
{
	//std::osyncstream bout(std::cout);
	//std::osyncstream syncstr{ str };
	//std::ostringstream str{ };
	//std::cout << heureDebut() << "Hello World!\n";
	//std::syncstr << heureDebut() << "Hello World!\n";
	//str << heureDebut() << "Hello World!\n";
	//std::cout << str.str();
	//affiche(str << heureDebut() << "Hello World!\n");

	Heure* heureCourante, * heureLimite;

	heureCourante = getHeure();
	heureLimite = initialise(argc, argv);

	
	afficheDate();
	affiche(heureCourante);
	affiche(heureLimite);

	if (!limiteDepasse(heureCourante, heureLimite)) {

		long duree = difference(heureCourante, heureLimite);
		if (duree == -1) {
			std::cerr << heureDebut() << "pb de calcul de la duree" << duree << "\n";
			exit(1);
		}

		std::string fichier = "";
		if (argc >= 4) {
			fichier = argv[3];
		}

		std::thread* t1;

		if (fichier.size() > 0) {
			//std::cout << heureDebut() << "démarrage du thread ...\n";
			AFFICHE("démarrage du thread ...");
			//std::thread t1(task1, fichier);
			t1 = new std::thread(task1, fichier);
			//std::cout << heureDebut() << "démarrage du thread ok\n";
			//m_screen.lock();
			AFFICHE("démarrage du thread ok");
			//m_screen.unlock();
		}

		//std::cout << heureDebut() << "attente"<< duree <<" secondes\n";
		//m_screen.lock();
		//str.clear();
		//str << heureDebut() << "attente" << duree << " secondes\n";
		//std::cout << str.str();
		AFFICHE("attente " << duree << " secondes");
		//m_screen.unlock();
		Sleep(duree);

		//std::cout << heureDebut() << "arret de l'ordinateur ...\n";
		//m_screen.lock();
		//str.clear();
		//str << heureDebut() << "arret de l'ordinateur ...\n";
		//std::cout << str.str();
		AFFICHE("arret de l'ordinateur ...");
		//m_screen.unlock();

		arret();
	}
	else {
		std::cout << heureDebut() << "fin\n";
	}

}

// Exécuter le programme : Ctrl+F5 ou menu Déboguer > Exécuter sans débogage
// Déboguer le programme : F5 ou menu Déboguer > Démarrer le débogage

// Astuces pour bien démarrer : 
//   1. Utilisez la fenêtre Explorateur de solutions pour ajouter des fichiers et les gérer.
//   2. Utilisez la fenêtre Team Explorer pour vous connecter au contrôle de code source.
//   3. Utilisez la fenêtre Sortie pour voir la sortie de la génération et d'autres messages.
//   4. Utilisez la fenêtre Liste d'erreurs pour voir les erreurs.
//   5. Accédez à Projet > Ajouter un nouvel élément pour créer des fichiers de code, ou à Projet > Ajouter un élément existant pour ajouter des fichiers de code existants au projet.
//   6. Pour rouvrir ce projet plus tard, accédez à Fichier > Ouvrir > Projet et sélectionnez le fichier .sln.
