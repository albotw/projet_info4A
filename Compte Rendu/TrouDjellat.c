#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

//! GESTION MéMOIRE ====================================================================
#define maxSize 50000   //? Taille max en Octets
#define maxBlocks 5000  //? Nombre max de blocs allouables.

typedef struct{
    void* adr;
    unsigned size;
}block;

unsigned dmSize;    //? taille actuelle en octets du tas
unsigned nbBlocks;  //? nb de blocs instanciés dans le tas
block** blocks;   //? Tableau référençant tous les blocs mémoire crées.

int verbose;    //? afficher les retours console ou non.

int find(int mode, void* adr);
//? mode == 0 ~> recherche du premier emplacement vide
//? mode == 1 ~> recherche de l'adresse.

void initDMM(int verboseMode);

void* _malloc(unsigned size);

void _free(void* adr);

//! UTILITAIRE =============================================================

void println(); //? affiche un retour a la ligne dans la console.

void updateRandomSeed();    //? met a jour la graine pour l'aléatoire.

int RandomizedInt(int a, int b);
//? B > A && return E [a;b]

//! MAIN ===================================================================
typedef struct{
    char N;
    char E;
    char S;
    char W;
}piece;

piece* tab;

int cote;   
//? taille du côté du tableau

int nbPieces;

void generateTab(int size, int mode); 
//? mode 0 = mode simple, mode 1 = mode complexe

void rotate(int posX, int posY);   

void swap(int x1,int y1, int x2, int y2); 

piece getPieceAt(int x, int y); 

void setPieceAt(int x, int y, piece p); 

void draw(); 

int checkConflicts();   

char generateFaceFromContext(int x, int y, char face);

char* formatChar(char c);   

int choixTailleTableau();
//? Interface

int rejouer();
//? Interface


//! GESTION TAS ============================================================
int find(int mode, void* adr) //? utiliser rechercher dichotomique
{
    int output = -1;
    if (mode == 0)
    {
        do
        {
            output++;
        }while(blocks[output] != NULL && output < maxBlocks);
    }
    else if (mode == 1)
    {
        if (adr != NULL){
            int found = 0;
            for(int i = 0; i < nbBlocks; i++)
            {
                void* ptr = blocks[i]->adr;
                if (found == 0 && ptr == adr)
                {
                    output = i;
                    found = 1;
                }
            }
        }
    }
    return output;
}

void* _malloc(unsigned size)
{
    if (dmSize + size <= maxSize && nbBlocks + 1 <= maxBlocks)
    {
        dmSize = dmSize + size;
        nbBlocks++;

        block* b = calloc(1, sizeof(block));
        b->adr = calloc(1, size);
        b->size = size;

        int position = find(0, NULL);
        blocks[position] = b;
        
        if (verbose) printf("Allocated %u bytes @ %p / %d available\n", b->size, b->adr, maxSize - dmSize);

        return b->adr;
    }
    else
    {
        if (verbose) printf("Error: dynamic memory saturated\n");
        
        return NULL;
    }
}

void _free(void* adr)
{
    
    int position = find(1, adr);
    if (position != -1)
    {
        block* b = blocks[position];
        dmSize = dmSize - b->size;
        nbBlocks--;

        if (verbose) printf("Released %u bytes @ %p / %d available\n", b->size, b->adr, maxSize - dmSize);
        
        free(b->adr);
        free(blocks[position]);
        blocks[position] = NULL;
    }
    else
    {
        if (verbose) printf("Error: invalid adress\n");
    }
}

void initDMM(int verboseMode)
{
    verbose = verboseMode;
    blocks = calloc(maxBlocks, sizeof(block));
}

//! UTILITAIRE ===========================================================

void println()
{
    printf("\n");
}

void updateRandomSeed()
{
    srand(time(NULL));
}

int RandomizedInt(int a, int b)
{
    b++;
    return (rand() % (b - a)) + a;
}

//! MAIN ===================================================================
int checkConflicts()
{
    int conflits = 0;
    for(int i = 0; i < cote; i++)
    {
        for (int j = 0; j < cote; j++)
        {
            piece p = getPieceAt(j, i);

            if (getPieceAt(j - 1, i).E != p.W)
            {
                conflits++;
            }
            if (getPieceAt(j, i - 1).S != p.N)
            {
                conflits++;
            } 
            if (getPieceAt(j + 1, i).W != p.E){
                conflits++;
            }
            if (getPieceAt(j, i + 1).N != p.S)
            {
                conflits++;
            }
        }
    }
    return conflits / 2;
}

char generateFaceFromContext(int x, int y, char face)
{
    char temp = '\0';
    switch(face)
    {
        case 'N':
            temp = getPieceAt(x, y-1).S;
            break;

        case 'S':
            temp = getPieceAt(x, y+1).N;
            break;

        case 'E':
            temp = getPieceAt(x+1, y).W;
            break;

        case 'W':
            temp = getPieceAt(x-1, y).E;
            break; 
    }

    if (temp != '\0')
    {
        return temp;
    }
    else 
    {
        return 'A'+RandomizedInt(0, 3);
    }
}

void generateTab(int size, int mode)
{
    printf("####  Génération du tableau en cours  ####\n");
    
    nbPieces = size * size;
    cote = size;

    tab = (piece* )_malloc(nbPieces * sizeof(piece));
    printf("   Taille: %d x %d, %d pièces, %ld octets en mémoire\n", cote, cote, nbPieces, nbPieces * sizeof(piece));

    for (int i = 0; i < cote; i++)
    {
        for (int j = 0; j < cote; j++)
        {
            piece p =
            {
                generateFaceFromContext(j, i, 'N'),
                generateFaceFromContext(j, i, 'E'),
                generateFaceFromContext(j, i, 'S'),
                generateFaceFromContext(j, i, 'W')
            };
            
            setPieceAt(j, i, p);
        }
    }
    printf("   Tableau résolu généré: %d conflits\n", checkConflicts());

    int nbMoves = RandomizedInt(5, 50);
    int rotations = 0;
    int swaps = 0;

    for (int i = 0; i < nbMoves; i++)
    {
        int x = RandomizedInt(0, cote -1);
        int y = RandomizedInt(0, cote -1);

        int action = RandomizedInt(0, mode);

        if (action == 0)
        {
            rotate(x, y);
            rotations++;
        }
        else if (action == 1)
        {
            int x2 = RandomizedInt(0, cote -1);
            int y2 = RandomizedInt(0, cote -1);

            swap(x, y, x2, y2);
            swaps++;
        }
    }
    printf("   Fin du mélange: %d mouvements dont %d rotations et %d échanges\n", nbMoves, rotations, swaps);
    printf("####  Fin de la génération du tableau  ####\n");
    println();
}



piece getPieceAt(int x, int y)
{
    if (x < 0)
    {
        x = cote -1;
    }
    else if (x > cote - 1)
    {
        x = 0;
    }

    if (y < 0)
    {
        y = cote - 1;
    } 
    else if (y > cote - 1)
    {
        y = 0;
    }
    return tab[(x + (cote * y))];
}

void setPieceAt(int x, int y, piece p)
{
    tab[(x + (cote * y))] = p;
}

void swap(int x1, int y1, int x2, int y2)
{
    piece temp = getPieceAt(x1, y1);
    piece temp2 = getPieceAt(x2, y2);
    setPieceAt(x1, y1, temp2);
    setPieceAt(x2, y2, temp);    
}

void rotate(int x, int y)
{
    piece origine = getPieceAt(x, y);
    piece new = {origine.W, origine.N, origine.E, origine.S};
    setPieceAt(x, y, new);
}

char* formatChar(char c)
{
    char* S = _malloc(25 * sizeof(char));
    switch (c)
    {
        case 'A':
        {
            strcpy(S, "\033[48;5;9m\033[38;5;0mA\033[0m");
            break;
        }
        case 'B':
        {
            strcpy(S, "\033[48;5;13m\033[38;5;0mB\033[0m");
            break;
        }
        case 'C':
        {
            strcpy(S, "\033[48;5;11m\033[38;5;0mC\033[0m");
            break;
        }
        case 'D':
        {
            strcpy(S, "\033[48;5;14m\033[38;5;0mD\033[0m");
            break;
        }
    }

    return S;
}

void draw()
{
    for (int i = 0; i < cote; i++)
    {
        char top[(30*cote)];
        char mid[(60*cote)];
        char bot[(30*cote)];

        top[0] = '\0';
        mid[0] = '\0';
        bot[0] = '\0';

        sprintf(top+strlen(top), "   ");
        sprintf(mid+strlen(mid), " %d ",i); 
        sprintf(bot+strlen(bot), "   ");

        if (i == 0)
        {
            printf("   ");
        }

        char* c = NULL;
        for (int j = 0; j < cote; j++)
        {   
            if (i == 0)
            {
                printf("  %c   ", 'A' + j);
            }

            piece p = getPieceAt(j, i);

            c = formatChar(p.N);
            sprintf(top + strlen(top), "  %s   ", c);
            _free(c);

            c = formatChar(p.W);
            sprintf(mid + strlen(mid), "%s # ", c);
            _free(c);

            c = formatChar(p.E);
            sprintf(mid + strlen(mid), "%s ", c);
            _free(c);

            c = formatChar(p.S);
            sprintf(bot + strlen(bot), "  %s   ", c);
            _free(c);
        }
        
        if (i == 0){println();}

        printf("%s\n", top);
        printf("%s\n", mid);
        printf("%s\n", bot);
    }
}

void readCommand()
{
    //printf("Coordonnées possibles: x = [%c,%c] & y = [%d,%d]\n",'A', 'A' + cote -1, 0, cote - 1);
    println();
    printf("usage: XY ~> rotation | X1Y1X2Y2 ~> echange | ctrl+c ~> quitter\n");

    char* command = calloc(5, sizeof(char));
    scanf("%s", command);

    if (strlen(command) == 2)
    {
        int x = command[0] - 'A';
        int y = command[1] - '0';
        printf("Rotation: %d|%d %c|%c\n", x, y, command[0], command[1]);
        if (0 <= x && x <= cote && 0 <= y && y <= cote)
        {
            rotate(x, y);
        }
        else
        {
            printf("Erreur: coordonnées invalides\n");
        }
    }
    else if(strlen(command) == 4)
    {
        int x1 = command[0] - 'A';
        int y1 = command[1] - '0';
        int x2 = command[2] - 'A';
        int y2 = command[3] - '0';

        printf("Swap: %d|%d ~> %d|%d %c|%c ~> %c|%c\n", x1, x2, y1, y2, command[0], command[1], command[2], command[3]);
        if (0 <= x1 && x1 <= cote && 0 <= y1 && y1 <= cote && 0 <= x2 && x2 <= cote && 0 <= y2 && y2 <= cote)
        {
            swap(x1, y1, x2, y2);
        }
        else
        {
            printf("Erreur: coordonnées invalides \n");
        }
    }
    else
    {
        printf("Erreur: commande non reconnue \n");
    }
    free(command);
}

int main()
{
    updateRandomSeed();
    initDMM(0);
    int restart;

    do
    {
        restart = 0;
        int taille = choixTailleTableau();
        generateTab(taille + 3, 1);
        
        int victory;
        do{
            draw();
            victory = checkConflicts();
            println();
            printf("Il y a \033[48;5;9m%d\033[0m conflits restants\n", victory);
            readCommand();
            
            println();
        }while(victory != 0);
        
        
        restart = rejouer();
        if (restart)
        {
            _free(tab);
            tab = NULL;
        }
    }
    while(restart);
    return 0;
}

int choixTailleTableau()
{
    int y;
    int loop = 0;
    do
    {
        loop = 0;
        printf("Quelle taille de tableau allez vous prendre ?\n");
        printf("1: 4x4 ");
        printf("2: 5x5 ");
        printf("3: 6x6 ");
        printf("4: 7x7 ");
        println();
        scanf("%d", &y);

        if (1 > y || y > 4)
        {
            loop = 1;
            printf("Erreur: option invalide\n");
        }
    } while (loop);
    println();
    return y;
}

int rejouer()
{
    int temp;
    printf("Rejouer ?\n");
    printf("1: Oui\n");
    printf("2: Non\n");
    scanf("%d", &temp);

    if (temp == 2)
    {
        temp = 0;
    }
    return temp;
}