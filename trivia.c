#include<ncurses.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>

#define WIDTH 150  //pentru latimea ferestrei
#define HEIGHT 60 //pentru inaltimea ferestrei

typedef struct question
{
    char *text;///textul intrebarii
    char *var1;  ///variantele de raspuns
    char *var2;
    char *var3;
    char *var4;
    char *raspuns;///litera corespunzatoare raspunsului corect
    char *hint;
}question;

typedef struct date
{
    question *intrebari;
    int nr;//numarul de intrebari continute in joc(preluate din fisiere)
}date;

typedef struct 
{
    char *nume;
    int punctaj;
    struct tm tm;
}
istoric;

void print_menu (WINDOW *menu_win, int alegere, char *choices[], int n_choices) 
{ 
    int x=10, y=7, i=0;    
    while(i < n_choices) 
    {   
        if(alegere==(i+1) && strcmp(choices[i]," ")!=0)///daca optiunea care urmeaza a fi afisata este cea aleasa de utilizator, aceasta se afiseaza fiind marcata prin highlight pe scris
        {   
            wattron(menu_win, A_REVERSE);  
            mvwprintw(menu_win,y,x,"%s", choices[i]); 
            wattroff(menu_win,A_REVERSE);        
        }                                                                                                       
        else if(strcmp(choices[i]," ")!=0)       
        {
            start_color();
            init_pair(3,COLOR_BLUE,COLOR_BLACK);
            wattron(menu_win,COLOR_PAIR(3));///este folosita culoarea albastru pentru text pe un fundal negru
            mvwprintw(menu_win,y,x,"%s", choices[i]); //este afisata optiunea i din lista memorata in matricea char *choices[]
            wattroff(menu_win,COLOR_PAIR(3));
        }
        else
        {
            mvwprintw(menu_win,y,x,"----------------------------------"); ///aceasta este afisarea care se face pt variantele de raspuns eliminate in cadrul optiunii 50/50
    	    alegere++;
        }
        y=y+2;//urmatoarea optiune este afisata 2 linii la jos, astfel incat intre 2 optiuni sa existe o linie goala 
    	i=i+1; 
    }   
    wrefresh(menu_win);        
} 

int meniu ()
{
    WINDOW *menu_win;
    char *choices[] = {"New Game","Resume Game","View score list","Quit"}; 
    int highlight=1;//implicit este selectata prima optiune(optiunea curenta este marcata prin highlight pe scris)
    int choice=0,n_choices=4,a;//choice memoreaza indicele alegerii facute de utilizator
    initscr();//initializeaza modul curses
    noecho();///caracterele introduse nu vor fi afisate
    cbreak();//pastarea liniei in buffer este dezactivata
    int startx,starty;
    startx = (170 - WIDTH) / 2; 
    starty = (70 - HEIGHT) / 2; 
    menu_win = newwin(HEIGHT, WIDTH, starty, startx); 
    keypad(menu_win,TRUE);///permite utilizatorului sa foloseasca tastele sageti in interactiunea cu jocul
    print_menu(menu_win, highlight,choices,n_choices); 
    while(1)        
    {      
        a=wgetch(menu_win); //memoreaza tasta apasata
        switch(a)         
        {       
            case KEY_UP:        
                if(highlight == 1)//daca optiunea curenta este prima din lista si se apasa KEY_UP se face salt la ultima optiune       
                    highlight = n_choices;        
                else        
                    highlight=highlight-1;        
                break;        
            case  KEY_DOWN:        
                if(highlight == n_choices)///daca optiunea curenta este ultima din lista si se apasa KEY_DOWN se face salt la prima optiune       
                    highlight= 1;        
                else                               
                    highlight=highlight+1;        
                break;  
            case 10:///pentru apasarea tastei enter, se memoreaza in variabila choice alegerea curenta
                choice=highlight;
                break;                          
        }
        print_menu(menu_win,highlight,choices,n_choices);   
        if(choice != 0) //utilizatorul a ales una dintre optiuni
            break;        
    }                           
    wclear(menu_win);
    wrefresh(menu_win);
    endwin();
    delwin(menu_win);
    return choice;///este returnat indicele optiunii alese de utilizator(1,2,3 sau 4)
}


void afisare_timp (WINDOW *joc)///afiseaza data si ora curenta in fereastra joc, textul avand culoarea cyan pe un fundal negru
{
    time_t t=time(NULL);
    struct tm tm=*localtime(&t);
    start_color();
    init_pair(1,COLOR_CYAN,COLOR_BLACK);
    wattron(joc,COLOR_PAIR(1));
    mvwprintw(joc,1,1,"Data si ora curenta: %d-%d-%d %.2d:%.2d:%.2d",tm.tm_mday,  tm.tm_mon+1 , tm.tm_year +1900, tm.tm_hour, tm.tm_min, tm.tm_sec);
    wattroff(joc,COLOR_PAIR(1));
    wrefresh(joc);
}

void continut_joc (WINDOW *joc, int SCOR, date *a, int i)///afiseaza in fereastra joc textul intrebarii, scorul curent si un text catre jucator pentru a-l ghida in interactiunea cu aplicatia
{
    afisare_timp(joc);
    mvwprintw(joc,15,1,"********SCOR: %d puncte***********",SCOR);
    mvwprintw(joc,19,1,"Pentru optiunea 50/50, apasa tasta H");
    mvwprintw(joc,18,1,"Pentru optiunea Skip the question apasa tasta N");
    if(strcmp(a->intrebari[i].hint,"nothing\n")!=0)
        mvwprintw(joc,21,1,"Pentru un mic hint grafic apasa tasta W sau w (apoi apasa x)");
    start_color();
    init_pair(2,COLOR_BLUE,COLOR_BLACK);
    wattron(joc,COLOR_PAIR(2));
    mvwprintw(joc,5,4,a->intrebari[i].text);
    mvwprintw(joc,7,6,"A.");
    mvwprintw(joc,9,6,"B.");
    mvwprintw(joc,11,6,"C.");
    mvwprintw(joc,13,6,"D.");
    wattroff(joc,COLOR_PAIR(2));
}

void final_joc (WINDOW *joc, int SCOR, date *a, int r_corecte, int r_gresite, char PERS[])///afiseaza la finalul jocului scorul obtinut, nr de raspunsuri corecte, nr de raspunsuri gresite
{
    mvwprintw(joc,5,1,"Jocul s-a incheiat! Scorul pe care l-ai obtinut este %d  ",SCOR);
    if(SCOR==(a->nr)*5)///atunci cand jucatorul a selectat din prima incercare varianta corecta pentru fiecare intrebare
        mvwprintw(joc,6,1,"Felicitari, %s!!! Esti o enciclopedie ambulanta!!! Ai raspuns corect la toate intrebarile din test",PERS);
    else if(r_gresite!=0)
    {  
        mvwprintw(joc,6,1,"Bravo, %s! Ai raspuns corect la %d intrebari",PERS,r_corecte);
        mvwprintw(joc,7,1,"Fii mai atent runda viitoare! Ai introdus %d raspunsuri gresite",r_gresite);
    }
    else if(SCOR==(a->nr)*5 && r_gresite==0)
        mvwprintw(joc,6,1,"Se pare ca variantele ajutatoare ti-au fost de folos. Ai raspuns corect la toate cele %d intrebari",a->nr);
    mvwprintw(joc,10,1,"Daca doresti sa revii in meniul aplicatiei, apasa tasta m");
    mvwprintw(joc,11,1,"Daca doresti sa iesi din aplicatie, apasa oricare alta tasta");
    wrefresh(joc);
}

char* autentificare (char *nume)///functia returneaza sirul de caractere introdus de utilizator la tastatura
{
    initscr();
    clrtoeol();
    echo();
    char text[]="Introdu numele, apoi apasa tasta ENTER  ";
    int max_x,max_y;
    getmaxyx(stdscr,max_y,max_x);
    mvprintw(max_y/2,(max_x-strlen(text))/2-2,"%s",text);//pozitioneaza mesajul catre centrul ecranului
    getstr(nume);///preia in sirul de caractere nume textul introdus de utilizator
    refresh();
    clrtoeol();
    endwin();
    return nume;
}

void plasare_date (istoric user)//scrie in fisierul score_list.txt informatiile despre jocul care s-a incheiat
{
    FILE *f=fopen("score_list.txt","a");///deschidem fisierul in modul append
    if(f==NULL)
       printf("[Eroare]:Fisierul score_list.txt nu a putut fi deschis\n");
    fprintf(f,"%s ---> %d puncte --->Data: %d-%d-%d %.2d:%.2d:%.2d\n",user.nume,user.punctaj,user.tm.tm_mday, user.tm.tm_mon + 1, user.tm.tm_year + 1900, user.tm.tm_hour, user.tm.tm_min, user.tm.tm_sec);
    fclose(f);
}

void view_score_list();

int cauta (int v[], int nr, int x)///cauta pe x in vectorul v cu nr componente
{
    int i;
    for(i=0; i<nr; i++)
        if(v[i]==x)
            return -1;
    return 0;
}

void generare_vector (int v[], int nr)
{
   int i,x;
    for(i=0; i<nr; i++)
    {  
        while(1)
        {
            x=rand()%nr;
            if(cauta(v,nr,x)==0)
            { 
                v[i]=x;
                break;
            }
        }
    } 
}

void joc_nou (date *a)//functia afiseaza fereastra aferenta optiunii New Game, preluand ca parametru vectorul de structuri in care sunt memorate intrebarile din joc
{
    istoric user;
    char name[100];
    user.nume=autentificare(name);
    WINDOW *joc;
    initscr();
    noecho();
    curs_set(0);///dezactiveaza afisarea cursorului
    int highlight=1;///implicit este selectata prima varianta de raspuns
    int n_choices=4;///numarul de variante de raspuns
    int c, i,ok1=0,ok2=0;//ok1 trece in 1 atunci cand s-a ales optiunea "skip this question", iar ok2 trece in 1 atunci cand s-a ales optiunea 50/50
    int SCOR=0, r_corecte=0,r_gresite=0;
    char A[100],B[100],C[100],D[100];
    int v[1000]={-1};///vector de indici generati aleator 
    for(i=0; i<a->nr; i++)///initializare vector cu -1
        v[i]=-1;
    srand(time(NULL));
    generare_vector(v,a->nr);
    for(i=0; i<a->nr; i++)
    {   
    	highlight=1;///la fiecare intrebare care urmeaza a se afisa va fi marcata implicit prin highlight pe scris prima varianta de raspuns
        strcpy(A,a->intrebari[v[i]].var1);
        strcpy(B,a->intrebari[v[i]].var2);
        strcpy(C,a->intrebari[v[i]].var3);
        strcpy(D,a->intrebari[v[i]].var4);
        char *choices[]={A,B,C,D};///A,B,C,D reprezinta variantele de raspuns
        joc = newwin(23,79, 0, 0);
        keypad(joc,TRUE);///ofera posibilitatea de a folosi sagetile pentru a trece de la o varianta la alta   
        cbreak();
        afisare_timp(joc);
        continut_joc(joc,SCOR,a,v[i]);
        print_menu(joc,highlight,choices,n_choices); ///afiseaza varaintele de raspuns pentru prima data
        wrefresh(joc); 
        while(1)        
        {   
            afisare_timp (joc);  //functia reafiseaza data si ora astfel incat ele apar actualizate la apasarea unei taste
            c=wgetch(joc); ///c memoreaza tasta apasata in fereastra joc      
            {    
                if(c==KEY_UP)        
                {   
                    if(highlight == 1)        
                        highlight = n_choices;        
                    else        
                        --highlight;        
                }     
                if(c== KEY_DOWN)        
                {   
                    if(highlight == n_choices)        
                        highlight= 1;        
                    else                
                        ++highlight;         
                }
                if(c==10) //pentru tasta ENTER se verifica daca raspunsul ales este cel corect
                {   
                    int r=0;//r memoreaza indicele raspunsului corect (1-A,2-B,3-C,4-D)
                    char rasp=*(a->intrebari[v[i]].raspuns);
                    {
                   	if(rasp=='A')
                        r=1;
                    if(rasp=='B')
                        r=2;
                    if(rasp=='C')
                        r=3;
                    if(rasp=='D')
                        r=4;
                    }
                    if(highlight==r)///indicele raspunsului ales(highlight) este cel corect
                    {
                        SCOR=SCOR+5;
                        r_corecte++;
                        break;//se face salt catre urmatoarea intrebare
                    }
                    else
                    {
                        r_gresite++;
                        SCOR=SCOR-10;
                        mvwprintw(joc,15,1,"********SCOR: %d puncte***********",SCOR);///se actualizeaza scorul
                        wrefresh(joc);
                    }
                }
                if(c=='Q')///pentru Q se face revenirea la meniul principal
                {   
                    wclear(joc);
                    wrefresh(joc);
                    endwin();
                    int var=meniu();///rezultatul apelului functiei meniu este memorat in variabila var pentru a evita apeluri repetate ale functiei la fiecare expresie conditionala:
                   if(var==2)///daca s-a ales optiunea Resume, se revine in fereastra joc
                   {
                    	clrtoeol();
                    	continut_joc(joc,SCOR,a,v[i]);///se afiseaza din nou intrebarea curenta
                    	wrefresh(joc);
                   }
                   if(var==1)//se incepe un joc nou
                   {
                   		SCOR=0;
                    	r_corecte=0;
                    	r_gresite=0;
                    	clrtoeol();
                   		i=0;
                    	joc_nou(a);
                    	break;
                   }
                   if(var==4)//se iese din aplicatie
                    	exit(0);
                   if(var==3)//pentru alegerea optiunii view score list (indisponibila in timpul jocului)
                   {
                   		initscr();
    					clrtoeol();
   						char text[]="Optiune indisponibila in timpul jocului! Apasa b/B pentru a reveni la joc ";
    					mvprintw(2,2,"%s",text);
    					wrefresh(stdscr);
    					char c=wgetch(stdscr);
    					if(c=='b' || c=='B')//se revine la joc
    					{
    						clrtoeol();
                    		continut_joc(joc,SCOR,a,v[i]);
                    		wrefresh(joc);	
    					}
    					else///se inchide aplicatia
    					{
    						endwin();
    						clrtoeol();
    						exit(0);
    					}
                   }

                }
                if(c=='N' && ok1==0)//optiunea Skip this question este aleasa pentru prima data in timpul jocului (ok1=0)
                {
                    ok1=1;//pentru a asigura ca optiunea este folosita doar o data in cadrul jocului
                    break;//se face salt catre urmatoarea intrebare din joc
                }
                if(c=='N' && ok1==1)//optiunea Skip this question este aleasa inca o data in timpul unui joc
                    mvwprintw(joc,17,1,"----AI FOLOSIT DEJA OPTIUNEA 'SKIP THIS QUESTION'----");
                { 
                if(c=='H' && ok2==0)//optiunea 50/50 este aleasa pentru prima data in cadrul unui joc
                {
                    int r1,r2,r;//r memoreaza indicele raspunsului corect(0-A,1-B,2-C,3-D); r1 si r2 reprezinta indicii variantelor care urmeaza a fi eliminate
                    char rasp=*(a->intrebari[v[i]].raspuns);
                    if(rasp=='A')
                        r=0;
                    if(rasp=='B')
                        r=1;
                    if(rasp=='C')
                        r=2;
                    if(rasp=='D')
                        r=3;
                    srand(time(NULL));
                    while(1)
                    {
                        r1=rand()%4;//un numar aleator de la 0 la 3
                        r2=rand()%4;
                        if((r1!=r && r2!=r) && r1!=r2)///raspunsurile eliminate sunt diferite de raspunsul corect si difera intre ele(daca sunt egale s-ar elimina o singura varianta)
                            break;
                    }
                    if(r1==0)
                    	strcpy(A," ");
                    if(r1==1)
                    	strcpy(B," ");
                    if(r1==2)
                    	strcpy(C," ");
                    if(r1==3)
                    	strcpy(D," ");
                    if(r2==0)
                    	strcpy(A," ");
                    if(r2==1)
                    	strcpy(B," ");
                    if(r2==2)
                    	strcpy(C," ");
                    if(r2==3)
                    	strcpy(D," ");
                    highlight=0;///inainte de a reafisa cele 2 variante ramase alegerea implicita este 0, adica nu va fi marcata initial la afisare niciuna dintre variante prin hightlight pe scris
                    print_menu(joc,highlight,choices,n_choices);
                    ok2=1;
                }
                else if(c=='H' && ok2==1)///optiunea 50/50 este aleasa inca o data in cadrul unui joc
                    mvwprintw(joc,16,1,"----AI FOLOSIT DEJA OPTIUNEA '50/50'----"); 
                if(c=='W' || c=='w')///se solicita un hint
                {
                    char buf[100];
                    sprintf(buf,"feh -x -g 640x480 %d.png",v[i]+1);
                    system(buf);//deschide imaginea prin comanda salvata in sirul buf(comanda foloseste utilitarul feh)
                }
            }
            }
        print_menu(joc,highlight,choices,n_choices);        
    }  
    }   
    wclear(joc); 
    final_joc(joc,SCOR,a,r_corecte,r_gresite,name);
    user.punctaj=SCOR;
    time_t t=time(NULL);
    struct tm tm=*localtime(&t);
    user.tm=tm;//data si ora curenta
    plasare_date(user);//plaseaza informatiile despre ultimul joc intr-un fisier pentru a putea fi accesate si la inchiderea aplicatiei
    char w=wgetch(joc);///c memoreaza caracterul introdus de jucator in fereastra joc
    werase(joc);  
    wrefresh(joc);
    delwin(joc);
    int var;
    if(w=='m')///dupa finalizarea jocului se poate reveni in meniu apasand tasta m
    {
        var=meniu();
        clrtoeol();
        if(var==1)//se incepe un nou joc
        {
            refresh();
            joc_nou(a);
        }
        else if(var==4)
            exit(0);
        else if(var==3)
            view_score_list(a);
    }
    else
    {
        clear();
       	clrtoeol();
        endwin();
        exit(0);
    }
}


date *preluare_intrebari(int argc,char *argv[])
{
    if(argc==1)
    {
        printf("[Eroare]: Nu s-au dat argumente in linia de comanda");
        exit(1);
    }   
    FILE *intrebari=NULL;//fiserele cu intrebari si raspunsuri
    char sir[100];///memoreaza linia curenta din fisierul cu intrebari
    int cap=3,i=-1,c=0,j=0;
    date* input;//variabila input este de tip date cu 2 campuri(primul de tip question si al doilea un int nr ce memoreaza nr de total de intrebari)
    input=(date* )malloc(sizeof(date));
    question *a;//vector de structuri alocat dinamic //care ca memora intrebarile si raspunsurile(se vor aloca dinamic si elementele din structura)
    question *tmp=NULL;
    a=(question*)malloc(cap*sizeof(question));//initial, spatiu alocta pt vector este de 5 intrebari
    for(c=1;c<=(argc-1);c++)
    {
        intrebari=fopen(argv[c],"r");   
        if(intrebari==NULL)
        {
            printf("[Eroare]: Fisierul %s nu poate fi deschis.",argv[c]);
            exit(1);
        }
        while(fgets(sir,100,intrebari)!=NULL)
        {
        i++;
        if(i%7==0)
            {
                a[j].text=strdup(sir);
                continue;
            }
        if(i%7==1)
            {
                a[j].var1=strdup(sir);
                continue;
            }
        if(i%7==2)
            {
                a[j].var2=strdup(sir);
                continue;
            }
        if(i%7==3)
            {
                a[j].var3=strdup(sir);
                continue;
            }
        if(i%7==4)
            {
                a[j].var4=strdup(sir);
                continue;
            }
        if(i%7==5)
        {
            a[j].raspuns=strdup(sir);
            continue;
            
        }
        if(i%7==6)
        {
            a[j].hint=strdup(sir);
            j++;//j reprezinta nr de intrebari din toate fisierele date ca argument adica nr de componente al vectorului de structuri
        }
        if(j==cap)
        {   
            cap=cap*2;
            tmp=(question *)realloc(a,cap*sizeof(question));
            if(tmp!=NULL)
                a=tmp;
        }
    }
    fclose(intrebari);
    }
    a=(question *)realloc(a,j*sizeof(question));
    input->intrebari=a;
    input->nr=j;
    return input;
}


void free_mem(date *a)
{
    int i=0;
    for(i=0;i<a->nr;i++)
    {
        free(a->intrebari[i].var1);
        free(a->intrebari[i].var2);
        free(a->intrebari[i].var3);
        free(a->intrebari[i].var4);
        free(a->intrebari[i].text);
        free(a->intrebari[i].raspuns);
        free(a->intrebari[i].hint);
    }
    free(a->intrebari);
    free(a);
}

void view_score_list (date *a)///afiseza continutul fisierului in care s-au stocat date despre jocurile precedente
{
    initscr();
    FILE *f;
    int y=0;
    char player[100];
    f=fopen("score_list.txt","r");
    if(f==NULL)
        printf("[Eroare]: Nu am putut deschide fisierul score_list.txt");
    else
    {
        while(fgets(player,100,f))///in sirul player este citita fiecare linie din fisier
        {
            mvwprintw(stdscr,y,1,"%s",player);
            y=y+1;
        }
        fclose(f);
    }
    printw("Daca vrei sa revii la meniu, apasa m sau M");
    char t;
    t=getch();///in t este memorata tasta apasate de jucator
    erase();  
    refresh();
    int var;
    if(t=='m' || t=='M')
    {	
      	var=meniu();
      	clrtoeol();
      	if(var==1)///se incepe un nou joc
      	{  
      		clrtoeol();
      		joc_nou(a);
      	}
      	if(var==3)///s-a ales optiunea view score list
      		view_score_list(a);
      	if(var==4)///s-a ales optiunea quit
      		exit(0);
  	}	
    endwin();
}


int main(int argc,char *argv[])
{
    srand(time(NULL));
    date *A=NULL;
    A=preluare_intrebari(argc,argv);
    int var=meniu();
   	if(var==1)
        joc_nou(A);
   	if(var==4)
    {
        free_mem(A);
        exit(0);
    }
   	if(var==3)
       	view_score_list(A);
    free_mem(A);
 
    return 0;         
} 