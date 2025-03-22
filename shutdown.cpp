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

//namespace fs = std::filesystem;

struct Heure {
    int heure;
    int minute;
    int seconde;
};

typedef struct Heure Heure;

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

    oss << newtime.tm_hour << ":" << newtime.tm_min << ":" << newtime.tm_sec<<" : ";

    mystr = oss.str();
    //std::string formatted_str = std::format(
    //    "My name is {1:.2s} and pi is {0:.2f}", 1, 2);

    //return std::format("{}:{}:{}", newtime.tm_hour, newtime.tm_min, newtime.tm_sec);
    return mystr;
}

Heure *getHeure() {
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

void affiche(Heure *heure) {
    if (heure != NULL) {
        std::cout << heureDebut() << "heure:" << heure->heure << ":" << heure->minute<<":"<<heure->seconde << "\n";
    }
}

Heure* initialise(int argc, char* argv[]) {
    Heure* resultat;
    int heure, minute;
    bool trouve = false;
    heure = 15;
    minute = 0;
    if (argc >= 3) {
        char *heureStr = argv[1];
        char* minuteStr = argv[2];
        int heure2 = strtol(heureStr, NULL, 10);
        int minute2= strtol(minuteStr, NULL, 10);
        if (heure2 > 0 && heure2 < 20 && minute2>0 && minute2 < 60) {
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

    std::cout << heureDebut() << "Arret ...\n";
    int res = ExitWindowsEx(EWX_POWEROFF, 0);
    if (res == 0) {
        std::cout << heureDebut() << "Arret OK\n";
    }
    else {
        std::cout << heureDebut() << "Arret erreur : "<<res<<"\n";
    }
    
}

bool limiteDepasse(Heure* heureCourante, Heure*heureLimite) {

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
    long h = heure->heure*60*60  +heure->minute*60 + heure->seconde;
    return h*1000;
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
    long h1=0, h2=0;
    h1 = dureeMSecondes(heureCourante);
    h2= dureeMSecondes(heureLimite);
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

std::string& getEtat(std::string fichier) {
    //if (fs::exists(fichier)) {

    //}

    std::ifstream infile(fichier);

    //std::ifstream myfile;
    //myfile.open("file.txt");
    std::string line;
    while (std::getline(infile, line))
    {
        std::cout << heureDebut() << "ligne: '"<<line<<"'\n";
    }

    std::string s= std::string();
    return s;
}

// The function we want to execute on the new thread.
void task1(std::string fichier)
{
    std::cout << "task1 says: " << fichier;

    std::string etat;
    //etat = getEtat(fichier);
    std::cout << heureDebut() << "etat: " << etat << "\n";
    std::cout << heureDebut() << "suite ...\n";
}

int main(int argc, char* argv[])
{
    std::cout<< heureDebut() << "Hello World!\n";

    Heure* heureCourante, * heureLimite;

    heureCourante = getHeure();
    heureLimite = initialise(argc,argv);

    std::cout << heureDebut() << "heure courante: "<<"\n";
    affiche(heureCourante);
    affiche(heureLimite);

    if (!limiteDepasse(heureCourante, heureLimite)) {
        
        long duree = difference(heureCourante, heureLimite);
        if (duree == -1) {
            std::cerr << heureDebut() << "pb de calcul de la duree"<< duree<<"\n";
            exit(1);
        }

        std::string fichier = "";
        if (argc >= 4) {
            fichier = argv[3];
        }

        if (fichier.size() > 0) {
            std::cout << heureDebut() << "démarrage du thread ...\n";
            std::thread t1(task1, fichier);
            std::cout << heureDebut() << "démarrage du thread ok\n";
        }

        std::cout << heureDebut() << "attente"<< duree <<" secondes\n";
        Sleep(duree);

        std::cout << heureDebut() << "arret de l'ordinateur ...\n";

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
