/**
 *@file yatzee.c
 *ANSI @c Text-Based Yahtzee(R) Game Implementation
 *@author @HipercubesHunter11 (GitHub)
 *
 *(C)Copyright 2017 @HipercubesHunter11 (in GitHub)
 *Licensed under the Apache License, Version 2.0 (the "License");
 *You may not use this file except in compliance with the License.
 *You may obtain a copy of the License at
 *   http://www.apache.org/licenses/LICENSE-2.0
 *Unless required by applicable law or agreed to in writing,
 *software distributed under the License is distributed on an
 *"AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 *either express or implied. See the License for the
 *specific language governing permissions and limitations under the
 *License.
 *
 *CHANGELOG
 *April 9 2017: Uploading in GitHub
 *May 6 2017: Portability improve:
 *  @code fflush(stdin); @endcode
 *  replaced for @code ibufclean(stdin); @endcode
 *! Yikes! Intoduced a bug: Needed two LF when should be one. Sorry
 *July 7 2017:
 *  + At possible using 70 chars per line, rm'd unnecesary spaces
 *    ("rm'd". Pffff... MegaLOL)
 *  + More javadoc in comments
 *  + 22 now is just the default NAME_LEN (by #ifdef guards)
 *    For things like:
 *      @code gcc -Wp,-DNAME_LEN=10 yatzee.c @endcode
 *    (Same to MAX_NPLAYERS=10U)
 *  + Verifying ANSI C (just in case)
 *  + Fixed weird bug of ibufclean. Yay!
 *  + Added @fn fgetline();
 *  + Dices are now shown horinzontally
 *  + One space at name promptin'
 *  + mv'd the "Select a game option" line
 *  + Merged opc table with score table
 *  + Added compilation date 'n' time
 */

/*Sanitizing*/
#if ! __STDC__
#error RUN FOR YOUR LIFE!!!
/*MegaLOL*/
#endif

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

#ifndef MAX_NPLAYERS
#define MAX_NPLAYERS 10U
#endif
#ifndef NAME_LEN
/*Looks too much
but if you're "Hipercube's Hunter 11!"?*/
#define NAME_LEN 22
#endif

/*Somewhere, enums are inapropiate*/
#define aces 0
#define twos 1
#define threes 2
#define fours 3
#define fives 4
#define sixes 5
#define eq3 6
#define eq4 7
#define fhouse 8
#define sstrght 9
#define lstrght 10
#define yatzee 11
#define chance 12
#define ubonus 13
#define ybonus 14

#ifdef	__cplusplus
extern "C" {
#endif

typedef unsigned char usmall_t;
typedef short int  Score_t;
typedef struct {
    Score_t Tscore;
    char Name[NAME_LEN+1/*'\0'*/];
} Player;

static Score_t ScoreTable[MAX_NPLAYERS][15];
static Player Ranking[MAX_NPLAYERS];
static unsigned int n_players;

static const char BoxChars[14]={'1','2','3','4','5','6',
                                't','q','f','s','l','y','c',
                                'r'};

static const char BoxNames[15][15]={
"Aces","Twos","Threes","Fours","Fives","Sixes",
"Three equal","Fours equal","Full House","Small Straight","Large Straight","Yatzee!","Chance",
"Upper Bonus","Yatzee Bonus"
};

/* only is pretty in mono-spaced terminals*/
static const char ASCII_Dices[6][4][4]={{"   "," 0 ","   "},
                                        {"0  ","   ","  0"},
                                        {"0  "," 0 ","  0"},
                                        {"0 0","   ","0 0"},
                                        {"0 0"," 0 ","0 0"},
                                        {"0 0","0 0","0 0"}};

int ibufclean(FILE* in);
/*In a nutshell, does anywhere what fflush(stdin); in Win
Really, cleanes the input buffer until \n (incluiding it)
and return discarded char's count*/
char* fgetline(char* buf,int len,FILE* in);
int ScoreCompare(const void*, const void*);
/*Comparer for ANSI @c qsort(); at the end of the game*/
Score_t Score(const usmall_t* counts, usmall_t Box_i);
/**@return the score of counts by Box_i
@param Box_i: One of the macro constants (excluding bonuses)
@param Counts: Counts[i] is the count of (i+1)'s in the dice*/
Score_t Table(const usmall_t* counts, unsigned int p_n);
/*Show the ScoreTable of player p_n,
and @return its total score
@param p_n: player index (from 0 to (n_players-1))
@param counts: if not Null it's used by calling for
Score_t Score(const usmall_t*,usmall_t)
and show it in empty boxes
i.e., previsualize possible scores for the current round*/
int dirseed(void);
/*Ask for (sizeof(int)/sizeof(char))
random lines of characters for making
a really random seed (for void srand(int))*/
int randint(int n);
/*With the help of ANSI @c rand();,
@param a random number between 0 and n-1
@note Uniformity of int rand(void)
guarantees uniformity of int randint(int)*/

int main(void){
    printf("%s",
"======================================================================\n"
"yatzee.c :\n"
"ANSI C Text-Based Yahtzee Game Implementation\n"
"Author: HipercubesHunter11 (GitHub)\n"
"COMPILED " __TIME__ " " __DATE__ "\n"
"______________________________________________________________________\n"
"                                              \n"
"Copyright 2017 @HipercubesHunter11 (in GitHub)\n"
"Licensed under the Apache License, Version 2.0 (the \"License\");\n"
"You may not use this file except in compliance with the License.\n"
"You may obtain a copy of the License at:\n"
"  http://www.apache.org/licenses/LICENSE-2.0\n"
"Unless required by applicable law or agreed to in writing,\n"
"software distributed under the License is distributed\n"
"on an \"AS IS\" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,\n"
"either express or implied. See the License for the specific language\n"
"governing permissions and limitations under the License.\n"
"======================================================================\n"
"Press Enter for continue >>>"
                );
    ibufclean(stdin);
    usmall_t round,rolls,j,k;
    unsigned int player_n;
    int i,opc_index;
    unsigned char opc;
    usmall_t dice[5]={0,0,0,0,0};
    do{
        printf("Enter number of players(max %u): ",MAX_NPLAYERS);
        scanf("%u",&n_players);
    }while((n_players==0)||(n_players>MAX_NPLAYERS));ibufclean(stdin);
    for(player_n=0;player_n<n_players;player_n++){
        /*For each player*/
        for(i=aces;i<ubonus;i++)ScoreTable[player_n][i]=(-1);
        /*-1 means empty, empty is not equal to 0*/
        ScoreTable[player_n][ubonus]=ScoreTable[player_n][ybonus]=0;
        /*the bonus' boxes don't need an empty different to 0*/
        printf("Player %u: Enter name(max 20 c.): ",(player_n+1U));
        fgetline(Ranking[player_n].Name,NAME_LEN+1,stdin);
    }
    for(round=0;round<13;round++){
        printf("Round %d:\n",round+1);
        srand(dirseed());
        for(player_n=0;player_n<n_players;player_n++){
            unsigned char flags[5]={'r','r','r','r','r'};
            /*indicates which dice'll be rolled, and which won't*/
            printf("%s\'s turn:\n",Ranking[player_n].Name);
            for (rolls=0;rolls<3;rolls++){
                usmall_t counters[6]={0,0,0,0,0,0};
                /* counts the dice by their number
                example: if you get one ace, and fours 3's,
                counters is {1,0,4,0,0,0}*/
                printf("Roll %d:\n",(int)(rolls+1));
                for (j=0;j<5;j++){
                    if (flags[j]=='r')
                        dice[j]=(usmall_t)randint(6);
                    counters[dice[j]]++;
                }
                puts("1: |2: |3: |4: |5: |");
                puts("--------------------");
                for (k=0;k<3;k++){
                    for (j=0;j<5;j++)
                        printf("%s|",ASCII_Dices[dice[j]][k]);
                    puts("");/*'\n'*/
                }
                Table(counters,player_n);
                puts("For Another Roll: [R/r]");
                do{
                    printf("Select a game option: ");
                    while(!isgraph((int) (opc= (char)tolower(getchar()) )));
                    /*ignore spaces and tabs*/
                    for (opc_index=0;opc_index<14;opc_index++)
                    /*searching algoritm*/
                        if (opc==BoxChars[opc_index])
                            break;
                    switch(opc_index){
                    case 13:
                        if (rolls==2)/*Last roll*/
                            puts("You don't have more rolls. Score in a box");
                        break;
                    case 14:
                        puts("Incorrect option. Choose again");
                        break;
                    default:
                        if (ScoreTable[player_n][opc_index]!=-1)/*marked box*/
                            puts("Box already scored. Score other box");
                    }
                }while((opc_index==14)
                       ||
                       ((opc_index==13)&&(rolls==2))
                       ||
                       ((ScoreTable[player_n][opc_index]!=-1)&&(opc_index!=14)&&(opc_index!=13)));
                if(opc!='r'){/*Scoring, not rolling*/
                    ScoreTable[player_n][opc_index]=Score(counters, opc_index);
                    printf("Scored: %d\n",ScoreTable[player_n][opc_index]);

                    /*When scored, the bonus are updated*/

                    int AuxSum=0;
                    for (i=aces;i<eq3;i++)/*goes around the upper section*/
                        if(ScoreTable[player_n][i]!=-1)/*marked box*/
                            AuxSum += ScoreTable[player_n][i];/*accumulate scores*/
                    if (AuxSum>=63)
                        ScoreTable[player_n][ubonus]=35;/*Upper bonus*/
                    if ((Score(counters,yatzee)==50)&&(ScoreTable[player_n][yatzee]==50))
                        ScoreTable[player_n][ybonus]+=100;/*Yahtzee Bonus*/
                    Ranking[player_n].Tscore=Table(NULL,player_n); /*show table and store new score*/
                    ibufclean(stdin);
                    printf("Press Enter for continue:>>");
                    getchar();
                    ibufclean(stdin);
                    break;/*For no more rolling*/
                }/*with break, who need an else?*/
                puts("Select Dice to Roll");
                puts("R/r to roll, k/K for keep");
                puts("Any other character\'ll be ignored");
                puts("Example:");
                puts("RkKKk for rolling only the 1st dice");
                for(j=0;j<5;j++)
                    do
                        flags[j]=(unsigned char)tolower(getchar());
                    while((flags[j]!='r')&&(flags[j]!='k'));
                }
        }
    }
    puts("End of game.");
    for(player_n=0;player_n<n_players;player_n++){
        printf("%s\'s Scoretable:\n",Ranking[player_n].Name);
        Table(NULL,player_n);
        printf("Press Enter for continue:>>");
        getchar();
        ibufclean(stdin);
    }
    puts("Comparing Scores...");
    qsort(Ranking,n_players,sizeof(Player),ScoreCompare);
    puts("Ranking Table:");
    for(player_n=0;player_n<n_players;player_n++)
        printf("\t%d. %s: %d\n", (int)(player_n+1), Ranking[n_players-player_n-1].Name, Ranking[n_players-player_n-1].Tscore);
    printf("Winner(s) with score %d:\n",Ranking[n_players-1].Tscore);
    for(player_n=0;player_n<n_players;player_n++)
        if(Ranking[player_n].Tscore==Ranking[n_players-1].Tscore)
            printf("\t%s\n",Ranking[player_n].Name);
    puts("Congrats!!");
    printf("Press Enter for continue:>>");
    getchar();
    ibufclean(stdin);
    return EXIT_SUCCESS;
}

int ScoreCompare(const void* a,const void* b){
    return (((const Player*)a)->Tscore)-(((const Player*)b)->Tscore);
}

Score_t Table(const usmall_t* counts, unsigned int p_n){
    int i,USum=0,LSum=0;
    puts("UPPER SECTION:");
    puts("__________________________");
    for(i=aces;i<eq3;i++)
        if(ScoreTable[p_n][i]!=-1){/*Marked box*/
            printf(" %s: %d\n",BoxNames[i],ScoreTable[p_n][i]);
            USum += ScoreTable[p_n][i];
        }else /*empty box*/
            if(counts!=NULL)/*Previsualization of possible score*/
                printf(" %s[%c]: (%d)\n",BoxNames[i],BoxChars[i],Score(counts,i));
            else
                printf(" %s: empty\n",BoxNames[i]);
    printf("Upper Subtotal: %d\n",USum);
    printf(" +Bonus: %d\n",ScoreTable[p_n][ubonus]);
    USum += ScoreTable[p_n][ubonus];
    printf("=Upper Total: %d\n",USum);
    puts("==========================");
    puts("LOWER SECTION:");
    puts("__________________________");
    for(i=eq3;i<ubonus;i++)
        if(ScoreTable[p_n][i]!=-1){/*Marked box*/
            printf(" %s: %d\n",BoxNames[i],ScoreTable[p_n][i]);
            LSum += ScoreTable[p_n][i];
        }else /*empty box*/
        if(counts!=NULL)/*Previsualization of possible score*/
            printf(" %s[%c/%c]: (%d)\n",BoxNames[i],BoxChars[i],toupper(BoxChars[i]),Score(counts,i));
        else
            printf(" %s: empty\n",BoxNames[i]);
    printf(" Yahtzee Bonus: %d\n",ScoreTable[p_n][ybonus]);
    LSum += ScoreTable[p_n][ybonus];
    printf("Lower Total: %d\n",LSum);
    puts("=======================");
    printf("Grand Total: %d\n",(USum+LSum));
    return(USum+LSum);
}

Score_t Score(const usmall_t* counts, usmall_t Box_i){
    usmall_t Aux_i,Aux_Sum=0;
    switch((int)Box_i){
    case aces:
    case twos:
    case threes:
    case fours:
    case fives:
    case sixes:
        return counts[Box_i]*(Box_i+1);
        /*Remember: with return, who need a break?*/
    case eq3:
        for(Aux_i=0;Aux_i<6;Aux_i++)
            if(counts[Aux_i]>=3)
                return Score(counts,chance);
        return 0;
    case eq4:
        for(Aux_i=0;Aux_i<6;Aux_i++)
            if(counts[Aux_i]>=4)
                return Score(counts,chance);
        return 0;
    case fhouse:/*yatzee is not full house, it's pretty fair*/
        for(Aux_i=0;Aux_i<6;Aux_i++)
        /*first search a number appeared exactly 3 times*/
            if(counts[Aux_i]==3)
                break;
        if(Aux_i!=6)
        /*if gets finded,
        then search a number appeared exactly 2 times*/
            for(Aux_i=0;Aux_i<6;Aux_i++)
                if(counts[Aux_i]==2)
                return 25;
        return 0;
    /*Direct (and proofed) logic formulas*/
    case sstrght:
        return (counts[2]&&counts[3]&&(
                                       (counts[0]&&counts[1])
                                       ||
                                       (counts[1]&&counts[4])
                                       ||
                                       (counts[4]&&counts[5])
                                       )
                )*30;
    case lstrght:
        return (counts[1]
                &&
                counts[2]
                &&
                counts[3]
                &&
                counts[4]
                &&
                (counts[0] || counts[5])
                )*40;
    case yatzee:
        for(Aux_i=0;Aux_i<6;Aux_i++)
            if(counts[Aux_i]==5)
                return 50;
        return 0;
    case chance:
        for(Aux_i=0;Aux_i<6;Aux_i++)
            Aux_Sum += counts[Aux_i]*(Aux_i+1);
        return Aux_Sum;
    default:
        return -1;
    }
}

int randint(int n){
/*modified version of int randint(int) in
URL: http://stackoverflow.com/questions/822323/how-to-generate-a-random-number-in-c/39475626#39475626
uniformity of rand() guarantees uniformity of randint(int)*/
    if(!n) return EXIT_FAILURE; /*HiperLOL*/
    if(n<0){
        if((-n)<0)
        /*weird, but try get -(-128) in complement-at-2 8 bits
        hint: it's -128!*/
            return EXIT_FAILURE; /*MegaLOL*/
        else
            return -(randint(-n));
    }
    if((n-1)==RAND_MAX)
    /*that means randint(int)'s services are not required*/
        return rand();
    int x; /*temporal variable*/
    while((x=rand())>=((RAND_MAX/n)*n));
    /*discards ((RAND_MAX + 1) %  n) values of rand(),
    usually incluiding RAND_MAX
    Remember: rand() has (RAND_MAX + 1) possible values*/
    return (x%n);
}

int dirseed(void){
    int seed=0;
    unsigned int i;
    unsigned char ch,b=0;
    printf("%d random lines:\n",(int)(sizeof(int)/sizeof(char)));
    for (i=0;i<sizeof(int);i++){
        while((ch=(unsigned char)getchar())!='\n')
            b+=ch;
        seed=(seed<<CHAR_BIT)|(int)(b);
    }
    return abs(seed);
}

int ibufclean(FILE* in){
    int c,i;
    for (i=0;(((c = fgetc(in)) != '\n') && (c != EOF));i++);
    return i;
}

char* fgetline (char* line, int len, FILE* in){
    char *tempp1,*tempp2;
    tempp1=fgets(line,len,in);
    if((tempp1!=NULL)
       &&
       ((tempp2=strchr(line,'\n'))!=NULL)
      ){
        *tempp2=(char)0;/*remove newlines*/
        ungetc('\n',in);
    }
    ibufclean(in);
    return tempp1;
}

#ifdef	__cplusplus
}
#endif
