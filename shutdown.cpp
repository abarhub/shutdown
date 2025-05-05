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
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <format>
#include <fstream>

#include <iostream>
#include <filesystem>
#include <chrono>
#include <ctime>

namespace fs = std::filesystem;

struct Heure {
	int heure;
	int minute;
	int seconde;
};

typedef struct Heure Heure;

struct SMode {
	Heure* heure;
	std::vector<int> joursSemaine;
};

typedef struct SMode SMode;

struct SFichier {
	std::string fichierLog;
	std::string fichierEtat;
	SMode* mode1;
	SMode* mode2;
	SMode* mode3;
};

typedef struct SFichier SFichier;

struct SInitialisation {
	Heure* heure;
	std::string fichierLog;
	std::string fichierEtat;
	bool finProgramme;
};

typedef struct SInitialisation SInitialisation;

#define ETAT_EN_COURS "en_cours"
#define ETAT_FIN "fin"
#define ETAT_NON_DEMARRE "non_demarre"
#define AFFICHE(sstream) { m_screen.lock(); std::ostringstream str{ }; str << heureDebut() << sstream; std::cout << str.str()<<std::endl; if (!fileLog.empty()) { std::ofstream myfile; myfile.open(fileLog, std::ios_base::app); myfile << str.str()<<std::endl; myfile.close();} m_screen.unlock(); }

const int version = 1;

const std::string LundiStr = "Lu";
const std::string MardiStr = "Ma";
const std::string MercrediStr = "Me";
const std::string JeudiStr = "Je";
const std::string VendrediStr = "Ve";
const std::string SamediStr = "Sa";
const std::string DimancheStr = "Di";

const int LundiNo = 1;
const int MardiNo = 2;
const int MercrediNo = 3;
const int JeudiNo = 4;
const int VendrediNo = 5;
const int SamediNo = 6;
const int DimancheNo = 0;

std::mutex m_screen;
std::string etatCourant;
std::string fileLog;

SFichier* fichierCourant = NULL;

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
		std::cerr << "Invalid argument to _localtime64_s.\n";
		exit(1);
	}

	//std::cout << std::format("The answer is {}.\n", 42);

	std::ostringstream oss;

	oss << std::setfill('0') 
		<< std::setw(2) << newtime.tm_hour << ":" 
		<< std::setw(2) << newtime.tm_min << ":"
		<< std::setw(2) << newtime.tm_sec << " : ";

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


// Fonction pour supprimer les espaces en début et fin de ligne
std::string trim(const std::string& str) {
	// Trouver le premier caractère non-espace
	auto start = std::find_if_not(str.begin(), str.end(), [](unsigned char ch) {
		return std::isspace(ch);
		});

	// Trouver le dernier caractère non-espace
	auto end = std::find_if_not(str.rbegin(), str.rend(), [](unsigned char ch) {
		return std::isspace(ch);
		}).base();

	// Retourner la sous-chaîne sans espaces
	return (start < end) ? std::string(start, end) : "";
}

int getNoJour(std::string jour) {
	if (jour == LundiStr) {
		return LundiNo;
	}
	else if (jour == MardiStr) {
		return MardiNo;
	}
	else if (jour == MercrediStr) {
		return MercrediNo;
	}
	else if (jour == JeudiStr) {
		return JeudiNo;
	}
	else if (jour == VendrediStr) {
		return VendrediNo;
	}
	else if (jour == SamediStr) {
		return SamediNo;
	}
	else if (jour == DimancheStr) {
		return DimancheNo;
	}
	else {
		return -1;
	}
}

std::vector<int> stringToVectorInt(const std::string& str) {
	std::vector<int> result;
	std::stringstream ss(str); // Utiliser stringstream pour faciliter la division
	std::string token;

	while (std::getline(ss, token, ',')) { // Lire jusqu'à la virgule
		// Supprimer les espaces blancs autour du token
		token.erase(std::remove_if(token.begin(), token.end(), ::isspace), token.end());

		try {
			result.push_back(getNoJour(token)); // Convertir en entier et ajouter au vecteur
		}
		catch (const std::invalid_argument& e) {
			std::cerr << "Erreur : impossible de convertir '" << token << "' en entier.\n";
			// Gérer l'erreur comme vous le souhaitez (par exemple, ignorer le token)
			exit(1);
		}
		catch (const std::out_of_range& e) {
			std::cerr << "Erreur : la valeur '" << token << "' est hors de portée pour un entier.\n";
			// Gérer l'erreur comme vous le souhaitez
			exit(1);
		}
	}

	return result;
}

// Fonction pour lire et traiter le fichier
SFichier* processFile(const std::string& filename) {
	std::ifstream file(filename);

	if (!file.is_open()) {
		std::cerr << "Impossible d'ouvrir le fichier : " << filename << std::endl;
		exit(1);
	}


	SFichier* fichier = new SFichier();
	fichier->fichierLog = "";
	fichier->mode1 = new SMode();
	fichier->mode1->heure = NULL;
	fichier->mode2 = new SMode();
	fichier->mode2->heure = NULL;
	fichier->mode3 = new SMode();
	fichier->mode3->heure = NULL;

	std::string line;
	while (std::getline(file, line)) {
		// Ignorer les lignes commençant par #
		if (!line.empty() && line[0] == '#') {
			continue;
		}

		// Supprimer les espaces de début et de fin
		line = trim(line);

		// Ignorer les lignes vides après le trim
		if (line.empty()) {
			continue;
		}


		// Séparer par le premier =
		size_t pos = line.find('=');
		if (pos != std::string::npos) {
			std::string key = trim(line.substr(0, pos));
			std::string value = trim(line.substr(pos + 1));

			std::cout << "Clé: '" << key << "', Valeur: '" << value << "'" << std::endl;
			if (key == "fichierLog") {
				fichier->fichierLog = value;
			}
			else if (key == "fichierEtat") {
				fichier->fichierEtat = value;
			}
			else if (key == "mode1" || key == "mode2" || key == "mode3") {
				SMode* mode=NULL;
				if (key == "mode1") {
					mode = fichier->mode1;
				}
				else if (key == "mode2") {
					mode = fichier->mode2;
				}
				else if (key == "mode3") {
					mode = fichier->mode3;
				}
				if (mode != NULL) {
					size_t pos2 = value.find(';');
					if (pos2 != std::string::npos) {
						std::string heure = trim(value.substr(0, pos2));
						std::string jours = trim(value.substr(pos2 + 1));

						size_t pos3 = heure.find('h');
						if (pos3 != std::string::npos) {
							std::string heure2 = trim(heure.substr(0, pos3));
							std::string minutes = trim(heure.substr(pos3 + 1));

							int heure3 = std::stoi(heure2);
							int minute2 = std::stoi(minutes);

							if (heure3 >= 0 && heure3 < 24 && minute2 >= 0 && minute2 < 60) {
								mode->heure = new Heure();
								mode->heure->heure = heure3;
								mode->heure->minute = minute2;
								mode->heure->seconde = 0;
							}
						}

						mode->joursSemaine = stringToVectorInt(jours);
					}
				}
			}
		}
	}

	file.close();

	return fichier;
}


SInitialisation* initialise(int argc, char* argv[]) {
	SInitialisation* resultat;
	Heure* resultatHeure;
	std::string fichierEtat = "";

	int heure, minute;
	bool trouve = false;
	bool finProgramme = false;
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
	else if (argc == 2) {

		std::string s = argv[1];

		AFFICHE("ficher:"<<s);

		SFichier* fichier = processFile(s);

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

		bool heureTrouve = false;
		int jourSemaine = newtime.tm_wday;
		AFFICHE("jour de la semaine:"<< jourSemaine);

		if (fichier != NULL) {
			int mode = -1;

			if (!fichier->fichierLog.empty()) {
				fileLog = fichier->fichierLog;
				AFFICHE("fichier de log" << fileLog);
			}

			if (!fichier->fichierEtat.empty()) {
				fichierEtat = fichier->fichierEtat;
			}

			if (fichier->mode1 != NULL) {
				for (int i = 0; i < fichier->mode1->joursSemaine.size(); i++) {
					if (jourSemaine == fichier->mode1->joursSemaine[i]) {
						mode = 1;
						break;
					}
				}
			}
			if (mode == -1&&fichier->mode2 != NULL) {
				for (int i = 0; i < fichier->mode2->joursSemaine.size(); i++) {
					if (jourSemaine == fichier->mode2->joursSemaine[i]) {
						mode = 2;
						break;
					}
				}
			}
			if (mode == -1 && fichier->mode3 != NULL) {
				for (int i = 0; i < fichier->mode3->joursSemaine.size(); i++) {
					if (jourSemaine == fichier->mode3->joursSemaine[i]) {
						mode = 3;
						break;
					}
				}
			}

			AFFICHE("mode:" << mode);
			Heure* heure2 = NULL;
			if (mode == 1) {
				heure2 = fichier->mode1->heure;
			}
			else if (mode == 2) {
				heure2 = fichier->mode2->heure;
			}
			else if (mode == 3) {
				heure2 = fichier->mode3->heure;
			}
			if (heure2 == NULL) {
				AFFICHE("heure:");
			}else {
				AFFICHE("heure:" << heure2->heure<<":"<<heure2->minute);
				heure = heure2->heure;
				minute = heure2->minute;	
				heureTrouve = true;
			}
		}

		AFFICHE("Fin");
		if (!heureTrouve) {
			AFFICHE("pas d'heure => pas d'attente, arret du programme");
			//exit(0);
			finProgramme = true;
		}
	}
	resultat = new SInitialisation();
	resultatHeure = new Heure();
	resultatHeure->heure = heure;
	resultatHeure->minute = minute;
	resultat->heure = resultatHeure;
	resultat->fichierEtat = fichierEtat;
	resultat->finProgramme = finProgramme;

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

bool isModifiedToday(const fs::path& filePath) {
	if (!fs::exists(filePath)) {
		AFFICHE("Le fichier n'existe pas.");
		return false;
	}

	// Récupère le temps de dernière modification
	auto ftime = fs::last_write_time(filePath);
	auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
		ftime - fs::file_time_type::clock::now()
		+ std::chrono::system_clock::now());

	AFFICHE("ftime:"<<ftime)
	AFFICHE("sctp:" << sctp)

	std::time_t file_c_time = std::chrono::system_clock::to_time_t(sctp);
	std::tm file_tm;
	localtime_s(&file_tm, &file_c_time);
	//std::tm file_tm = *std::localtime(&file_c_time);

	AFFICHE("file_c_time:" << file_c_time)

	// Récupère la date actuelle
	std::time_t now = std::time(nullptr);
	//std::tm now_tm = *std::localtime(&now);
	std::tm now_tm;
	localtime_s(&now_tm, &now);

	AFFICHE("now:" << now)

	return !(file_tm.tm_year == now_tm.tm_year &&
		file_tm.tm_mon == now_tm.tm_mon &&
		file_tm.tm_mday == now_tm.tm_mday);
}

std::string getEtat(std::string fichier) {

	fs::path file = fichier;
	if (isModifiedToday(file)) {
		return ETAT_NON_DEMARRE;
	}

	std::ifstream infile(fichier);

	std::string line;
	std::string s = std::string();
	while (std::getline(infile, line))
	{
		//AFFICHE("ligne: '" << line << "'");
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
			else if (etat.find(ETAT_NON_DEMARRE) != std::string::npos) {
				AFFICHE("backup non démarré");
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
	SInitialisation* initialisation = NULL;

	AFFICHE("version : " << version);

	//int nombre = 5;
	//std::cout << std::setfill('0') << std::setw(2) << nombre << std::endl;

	heureCourante = getHeure();
	initialisation = initialise(argc, argv);

	if (initialisation == NULL || initialisation->heure==NULL) {
		std::cerr << heureDebut() << "Erreur d'initialisation" << std::endl;
		exit(1);
	}
	heureLimite = initialisation->heure;
	
	afficheDate();
	affiche(heureCourante);
	affiche(heureLimite);

	if (!initialisation->finProgramme&&!limiteDepasse(heureCourante, heureLimite)) {

		long duree = difference(heureCourante, heureLimite);
		if (duree == -1) {
			std::cerr << heureDebut() << "pb de calcul de la duree" << duree << "\n";
			exit(1);
		}

		std::string fichier = "";
		if (argc >= 4) {
			fichier = argv[3];
		}
		else if (!initialisation->fichierEtat.empty()) {
			fichier = initialisation->fichierEtat;
		}

		AFFICHE("fichier etat : "<< fichier);


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
