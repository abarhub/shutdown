// shutdown.cpp : Ce fichier contient la fonction 'main'. L'exécution du programme commence et se termine à cet endroit.
//

#include <Windows.h>
#include <iostream>
#include <time.h>
#include <synchapi.h>

struct Heure {
    int heure;
    int minute;
    int seconde;
};

typedef struct Heure Heure;

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
        std::cerr << "Invalid argument to _localtime64_s.\n";
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
        std::cout << "heure:" << heure->heure << ":" << heure->minute<<":"<<heure->seconde << "\n";
    }
}

Heure* initialise(int argc, char* argv[]) {
    Heure* resultat;
    int heure, minute;
    bool trouve = false;
    heure = 15;
    minute = 0;
    if (argc == 3) {
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



void reboot() {

    std::cout << "Arret ...\n";
    int res = ExitWindowsEx(EWX_POWEROFF, 0);
    if (res == 0) {
        std::cout << "Arret OK\n";
    }
    else {
        std::cout << "Arret erreur : "<<res<<"\n";
    }
    
}

bool limiteDepasse(Heure* heureCourante, Heure*heureLimite) {

    if (heureCourante == NULL) {
        std::cerr << "heure courante null\n";
        exit(1);
    }
    if (heureLimite == NULL) {
        std::cerr << "heure limite null\n";
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
        std::cerr << "heure courante null\n";
        exit(1);
    }
    long h = heure->heure*60*60  +heure->minute*60 + heure->seconde;
    return h*1000;
}

long difference(Heure* heureCourante, Heure* heureLimite) {
    if (heureCourante == NULL) {
        std::cerr << "heure courante null\n";
        exit(1);
    }
    if (heureLimite == NULL) {
        std::cerr << "heure limite null\n";
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

int main(int argc, char* argv[])
{
    std::cout << "Hello World!\n";

    Heure* heureCourante, * heureLimite;

    heureCourante = getHeure();
    heureLimite = initialise(argc,argv);

    std::cout << "heure courante: "<<"\n";
    affiche(heureCourante);
    affiche(heureLimite);

    if (!limiteDepasse(heureCourante, heureLimite) >= 0) {
        
        long duree = difference(heureCourante, heureLimite);
        if (duree == -1) {
            std::cerr << "pb de calcul de la duree"<< duree<<"\n";
            exit(1);
        }
        std::cout << "attente"<< duree <<" secondes\n";
        Sleep(duree);

        std::cout << "arret de l'ordinateur ...\n";
    }
    else {
        std::cout << "fin\n";
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
