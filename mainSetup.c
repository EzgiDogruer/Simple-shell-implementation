//Ezgi Doğruer 150117042-İsra Nur Alperen 150117061-Elif Gökpınar 150117510

#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdint.h>
#include <dirent.h>

#define MAX_LINE 80 /* 80 chars per line, per command, should be enough. */
#define STATUS_RUNNING 0
#define STATUS_FINISHED 1
 
/* The setup function below will not return any value, but it will just: read
in the next command line; separate it into distinct arguments (using blanks as
delimiters), and set the args array entries to point to the beginning of what
will become null-terminated, C-style strings. */

//for signal handling:
int checkForegroundProcess = 0;
int currentFProcess;
int orderControl=1;
int sizeArgs=0;
int errorAddingRunningLL=0;
int orderBookmark;



//creates the queue for the background processes
typedef struct background_List {
    int status ;
    pid_t pid;
    char* command;
    int order;
    struct background_List* next;
} backgroundLists;

backgroundLists* backgroundLRunning = NULL;
backgroundLists* backgroundLFinished = NULL;

typedef struct bookmark_List{
	 char* command;
	 struct bookmark_List* next;
}bookmarks;

bookmarks* bookmark = NULL;


//////////////////////////////////////LINKEDLIST FUNCTIONS

//add a process to the ll
void addingBackground(pid_t pid, char* command, backgroundLists* backgroundList){
    if(backgroundList == NULL){ //add the first element
    	orderControl = 1;
        backgroundList = (backgroundLists*)malloc(sizeof(backgroundLists));
        backgroundList->command = (char*)malloc(sizeof(char)*strlen(command));
        strcpy(backgroundList->command, command);
        backgroundList->order = orderControl;
        backgroundList->pid = pid;
        backgroundList->next = NULL;
        orderControl++;
        backgroundList->status=0;
        backgroundLRunning=backgroundList;
        printf("[%d] Pid: [%d] Command: %s\n", backgroundLRunning->order , backgroundLRunning->pid, backgroundLRunning->command);
        return;
    }

    //find the end of the queue	
    backgroundLists* last = backgroundList;
    while(last->next != NULL){
        last = last->next;
    }

   //add to the end
    backgroundLists* new = (backgroundLists*)malloc(sizeof(backgroundLists));
    new->command = (char*)malloc(sizeof(char)*strlen(command));
    strcpy(new->command, command);
    new->order = orderControl;
    new->pid = pid;
    orderControl++;
    new->status=0;
    new->next = NULL;
    last->next = new;
    printf("[%d] Pid: [%d] Command: %s \n", new->order , new->pid, new->command);
    backgroundLRunning=backgroundList;
}
////////////////////////////////////////////////////////////////////////
//removes the element from the LL with given pid
void deleteByPid(int pid, backgroundLists* backgroundList){
	backgroundLists* killedProcess=(backgroundLists*)malloc(sizeof(backgroundLists));
    // remove the process from the background process queue
    if(backgroundList != NULL && backgroundList->pid == pid) {//If it's on the head of queue
         killedProcess = backgroundList;
         backgroundList = killedProcess->next;
         killedProcess->next = NULL;
         free(killedProcess);
         return;
    }
    else if(backgroundList != NULL) {//If it's not on the head of queue
         backgroundLists* iter = backgroundList;
         while(iter->next != NULL) {
              if(iter->next->pid == pid) {
                 killedProcess = iter->next;
                 killedProcess->next = iter->next;
                 killedProcess->next = NULL;
                 free(killedProcess);
                 return;
               }
          }
    }
}




//////////////////////////////////////////// running to finished linkedlist

void runningToFinished(int pid){
    
    if(backgroundLRunning != NULL && backgroundLRunning->pid == pid) {//If it's on the head of LL
         backgroundLists *changedProcess = backgroundLRunning;
         backgroundLRunning = changedProcess->next;// process runningden cÄ±ktÄ±
         if(backgroundLFinished==NULL) {// finished te hiÃ§ process yoksa process ekleme
         backgroundLFinished= changedProcess;
         changedProcess->next = NULL;
         }
         else {// finished te process varsa
         backgroundLists* iter2 = backgroundLFinished;
         while(iter2->next != NULL) {                     
             	iter2= iter2->next;
              }
              iter2->next=changedProcess;
              changedProcess->next = NULL;              
         }
         
         return;
    }
    else if(backgroundLRunning != NULL) {//If it's not on the head of queue
         backgroundLists* iter = backgroundLRunning;
         backgroundLists* changedProcess =NULL;
         backgroundLists* changedProcess2 =NULL;
         while(iter->next != NULL) {
              if(iter->next->pid == pid) {
                 changedProcess = iter->next;
                 changedProcess2 = changedProcess->next;
                 iter->next=changedProcess2;
                 if(backgroundLFinished==NULL) {
        	 backgroundLFinished= changedProcess;
        	 changedProcess->next = NULL;
        	 }
        	 else {// finished te process varsa
        	 backgroundLists* iter2 = backgroundLFinished;
         	 while(iter2->next != NULL) {                     
             	iter2= iter2->next;
            	  }
            	 
              	 iter2->next=changedProcess; 
              	 changedProcess->next = NULL;             
         }
               }
          }
    }
}

static void cathCtrlZ() {
   pid_t processID;
   processID = waitpid(-1,NULL,WNOHANG);
   //printf("The main process is not wait child anymore.\n");
}

void checkCtrlZ(){
    struct sigaction ctrlZ;
    ctrlZ.sa_handler = cathCtrlZ;
    ctrlZ.sa_flags = 0;
    if ((sigemptyset(&ctrlZ.sa_mask) == -1) ||
        (sigaction(SIGTSTP, &ctrlZ, NULL) == -1)) {
        perror("Failed to set SIGINT handler");
        exit(0);
    }
}


void addingBookmark(char *command){
	
	if(bookmark == NULL){ //add the first element
	orderBookmark=0;
	orderBookmark++;
        bookmark = (bookmarks*)malloc(sizeof(bookmarks));
        bookmark->command = (char*)malloc(sizeof(char)*strlen(command));
        strcpy(bookmark->command, command);
        bookmark ->next = NULL;
	
        return;
    }

    //find the end of the queue	
    bookmarks* last = bookmark;
    while(last->next != NULL){
        last = last->next;
    }
    
   //add to the end
    orderBookmark++;
    bookmarks* new = (bookmarks*)malloc(sizeof(bookmarks));
    new->command = (char*)malloc(sizeof(char)*strlen(command));
    strcpy(new->command, command);
    new->next = NULL;
    last->next = new;
   
}

void deleteBookmark(int element){
	bookmarks* iter=(bookmarks*)malloc(sizeof(bookmarks));
	iter=bookmark;
	int i=0;
	bookmarks* killedcommand=(bookmarks*)malloc(sizeof(bookmarks));
			

	if(element==0 && bookmark==NULL){
		printf("bookmark boş");
		return;
	}
	for(i=0;iter !=NULL;iter=iter->next,i++){
		
	}
	if(element >i){
		printf("bookmarkta böyle bir eleman yok");
		return;
	}
	//ilk elemanı ise
	if((bookmark != NULL)&& (element==0 )) {//If it's on the head of queue
         killedcommand = bookmark;
         bookmark = killedcommand->next;
         killedcommand->next = NULL;
         free(killedcommand);
	 orderBookmark--;
         return;
    	}

	iter=bookmark;
	for(i=0;i<element-1;i++,iter=iter->next){
		
	}
	
	killedcommand=iter->next;
        iter->next =killedcommand->next;
        killedcommand->next = NULL;
        free(killedcommand);
	orderBookmark--;

}

/////////////////////////////////////////////////////////////////////////////
void setup(char inputBuffer[], char *args[],int *background)
{
    int length, /* # of characters in the command line */
        i,      /* loop index for accessing inputBuffer array */
        start,  /* index where beginning of next command parameter is */
        ct;     /* index of where to place the next parameter into args[] */
    
    ct = 0;
        
    /* read what the user enters on the command line */
    length = read(STDIN_FILENO,inputBuffer,MAX_LINE);  

    /* 0 is the system predefined file descriptor for stdin (standard input),
       which is the user's screen in this case. inputBuffer by itself is the
       same as &inputBuffer[0], i.e. the starting address of where to store
       the command that is read, and length holds the number of characters
       read in. inputBuffer is not a null terminated C-string. */

    start = -1;
    if (length == 0)
        exit(0);            /* ^d was entered, end of user command stream */

/* the signal interrupted the read system call */
/* if the process is in the read() system call, read returns -1
  However, if this occurs, errno is set to EINTR. We can check this  value
  and disregard the -1 value */
    if ( (length < 0) && (errno != EINTR) ) {
        perror("error reading the command");
	exit(-1);           /* terminate with error code of -1 */
    }

	//printf(">>%s<<",inputBuffer);
    for (i=0;i<length;i++){ /* examine every character in the inputBuffer */

        switch (inputBuffer[i]){
	    case ' ':
	    case '\t' :               /* argument separators */
		if(start != -1){
                    args[ct] = &inputBuffer[start];    /* set up pointer */
		    ct++;
		}
                inputBuffer[i] = '\0'; /* add a null char; make a C string */
		start = -1;
		break;

            case '\n':                 /* should be the final char examined */
		if (start != -1){
                    args[ct] = &inputBuffer[start];     
		    ct++;
		}
                inputBuffer[i] = '\0';
                args[ct] = NULL; /* no more arguments to this command */
		break;

	    default :             /* some other character */
		if (start == -1)
		    start = i;
                if (inputBuffer[i] == '&'){
		    *background  = 1;
                    inputBuffer[i-1] = '\0';
		}
	} /* end of switch */
     }    /* end of for */
     args[ct] = NULL; /* just in case the input line was > 80 */

	for (i = 0; i <= ct; i++){
		//printf("args %d = %s\n",i,args[i]);
		}
	sizeArgs=i-1;
} /* end of setup routine */


void bookmarkCommand(char *args[]){
	int i=0;
	char str[128]="";
	char args1[128]="";
	strcpy(args1,args[1]);
	
	if(strcmp(args1,"-i")==0){

		char **res=NULL;
		bookmarks* iter=(bookmarks*)malloc(sizeof(bookmarks));
		iter=bookmark;
		for(i=0;i<(atoi(args[2]));i++,iter=iter->next){
			
		}
		
		
		char str[128]="";
		strcpy(str,iter->command);
		char *  p    = strtok (str, " ");
		int n_spaces = 0, i;


		/* split string and append tokens to 'res' */
	
		while (p) {
  			res = realloc (res, sizeof (char*) * ++n_spaces);

  			if (res == NULL)
    				exit (-1); /* memory allocation failed */

  			res[n_spaces-1] = p;

  			p = strtok (NULL, " ");
		}

		/* realloc one extra element for the last NULL */

		res = realloc (res, sizeof (char*) * (n_spaces+1));
		res[n_spaces] = 0;

		
		

		pid_t childpid;
  	       	childpid = fork();
   		if (childpid == -1) {
      		perror("Failed to fork");
      		return ;
   		}
   		if (childpid == 0) {                                      /* child code */
     			execvp(res[0],&(res[0]));
      			perror("Child failed to execvp the command");
     			 return ;
   		}
   		if (childpid != wait(NULL)) {                          /* parent code */
      			perror("Parent failed to wait");
      			return ;
   		}

	}



	else if (strcmp(args1,"-l")==0){


		bookmarks* iter=(bookmarks*)malloc(sizeof(bookmarks));
		iter=bookmark;
		//printf(" order: %d",orderBookmark);
		for(i=0;i<orderBookmark;i++,iter=iter->next){
			printf("\n%d \"%s\"\n",i,iter->command);
		}
		
	}
	else if(strcmp(args1,"-d")==0){
	
	  	deleteBookmark(atoi(args[2]));
	
	}
	
	else {
		
		for(i=1;i<sizeArgs-1;i++){
			strcat(str,args[i]);
			strcat(str," ");
			
		}//for
		strcat(str,args[i]);
		char *p = str;
		p++;
		p[strlen(p)-1] = 0;
		

		addingBookmark(p);
		
	}//else
	
	
}
//*************************SEARCH*********************************************

void trim(char *s)
{
	int i,j;
	for(i=0;s[i]==' '||s[i]=='\t';i++);
 
	for(j=0;s[i];i++)
	{
		s[j++]=s[i];
	}
	s[j]='\0';  
}
void findWord(FILE *readFile,char *fileName,char* word)
{
	int fileSize=0;int i=0;
	char *file_str,letter;	

	//Find how many character in the file.
	while(!feof(readFile)){
		fscanf(readFile,"%c",&letter);
		fileSize++;}
	
	file_str=(char*)malloc(sizeof(char)*fileSize);


	
	rewind(readFile);

        //take all characters
	while(!feof(readFile)){
		fscanf(readFile,"%c",&file_str[i]);
		i++;}
	
	char str[1000]=""; int index=0; int row=1; int column=0; int len=0; int findWordLen=strlen(word); int check=0; int add=0;
	for(index=0; index<fileSize; index++){
		
                if(file_str[index]!='\0')
                strncat(str,&file_str[index],1);
                
                
		if(file_str[index]=='\n'){
			
			//printf("satır %d %s",satir,str);
			len=strlen(str);
			
			for(column=0; column<len; column++){
			
			len=strlen(str);
			
			 if(word[0]==str[column]){
			 
			  for(check=1; check<findWordLen; check++){
			   if(column+check<len && word[check]==str[column+check]){
			  
			   add++;
			   }
			  } 
			  if(add==findWordLen-1){
			  trim(str);
			  printf("%d: ./%s => %s",row,fileName,str);
			  }
			}
			add=0;
			}
			row++;
			strcpy(str,"");
			
			
			}
		
	
			
	}
	//printf("ROW NUMBER %d ",row);

	free(file_str);	
}



void isFileExist(char *fileName,char *argv,int length){
             FILE *readFile;
            if((fileName[length-2]=='.' )&& (fileName[length-1]=='c' || fileName[length-1]=='C' || fileName[length-1]=='h' ||       fileName[length-1]=='H')){
              //printf("*%d* %s %c \n",length,fileName,fileName[length-1]);
                        
              //printf("***%s***",fileName);
	       
	       readFile =fopen(fileName,"r");
	       if (readFile==NULL){
               //printf("It is not exist\n");
               //strcpy(fileName,"");
	       }
               else{
        	findWord(readFile,fileName,argv); 
        	}
	       
		fclose(readFile);
                    
            }
    }
    
 void currentDirectory(char *argv2){
        
        int i=0; int length=0;
        char fileName[100]="";
       
            DIR *d;
            struct dirent *dir;
	     d = opendir(".");
            if (d){
            while ((dir = readdir(d)) != NULL){
            //printf("%s\n", dir->d_name);
            strcpy(fileName,dir->d_name);
            length=strlen(fileName);

            isFileExist(fileName,argv2,length);
            strcpy(fileName,"");
        }
        closedir(d);
    }


}

void subdirectoryRecursively(char *basePath,char *argv1)
{
    char path[1000];
    int length=0;
    
    struct dirent *dp;
    DIR *dir = opendir(basePath);

    // Unable to open directory stream
    if (!dir)
        return;

    while ((dp = readdir(dir)) != NULL)
    {
        if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0)
        {
            //printf("%s\n", dp->d_name);

            // Construct new path from our base path
            strcpy(path, basePath);
            strcat(path, "/");
            strcat(path, dp->d_name);
            length=strlen(path);
            isFileExist(path,argv1,length);

            subdirectoryRecursively(path,argv1);
        }
    }

    closedir(dir);
}

void search(char **args){

int i=0;
char *trim ="";
//printf("%d",sizeArgs);

        char property[10]="";
        strcpy(property,args[1]);
	if(sizeArgs!=2 && sizeArgs!=3){
	    printf("%s","Wrong input,try again.");
        }
			
       else if(sizeArgs==3 && strcmp("-r",property)==0){

         //printf("%s",property);
         trim= args[2];
         if(trim[0]=='"' && trim[strlen(trim)-1]=='"'){
	 trim++;
	 trim[strlen(trim)-1] = 0;
         subdirectoryRecursively("./",trim);
	 }
	 else{
	 printf("You should use quote each two side of the word!Try again.\n");
	 }
  
       
       }

	else if(sizeArgs==2){  
	 
	 trim= args[1];
	 if(trim[0]=='"' && trim[strlen(trim)-1]=='"'){
	 trim++;
	 trim[strlen(trim)-1] = 0;
         currentDirectory(trim);
	 }
	 else{
	 printf("You should use quote each two side of the word!Try again.\n");
	 }
	 
	 
	}
	else{
	printf("\nWrong input,try again.\n");
	}


}

//returns 1 if there is such a file
int checkFileExist( char *path) {

    FILE *file = fopen(path, "r");
    if (file != NULL){
    fclose(file);
    	return 1;}
        
        
    else
    return 0;
}

// signal should have no effect.
static void signalHandler() {
    int status;
    // If there is a foreground process
    if(checkForegroundProcess) {
        // kill current process
        kill(currentFProcess, 0);
        if(errno == ESRCH) { /*error check*/
            fprintf(stderr, "\nProcess %d not found\n", currentFProcess);
            checkForegroundProcess = 0;
            printf("myshell: ");
            fflush(stdout);
        }
        else{ /* if there is still foreground process*/
            kill(currentFProcess, SIGKILL);
            waitpid(currentFProcess, &status, WNOHANG);
            printf("\n");
            checkForegroundProcess = 0;
        }
    }
    // If there is no foreground process
    else{
        printf("\nmyshell: ");
        fflush(stdout);
    }
}


//finds the path of the executable
char** fullPath(char **args) {
    //splits path variable by : and copies into a paths array
    int k,i = 0;
    char getAllPath[1000]; 
    strcpy(getAllPath, getenv("PATH"));
    char token[] = ":";

    char *strToken = strtok(getAllPath, token);
    char *path[1000];
    int index = 0;

    while(strToken != NULL) {
        path[index] = (char *)malloc(sizeof(char)*100);
        strcpy(path[index], strToken);
        index++;

        strToken = strtok(NULL, token);
    }

    char *tempPath, **correctPathArray;

    tempPath = (char *)malloc(sizeof(char *) * 250);
    correctPathArray = (char **)malloc(sizeof(char *) * 250);
    for (k = 0; k < index; k++) {
        correctPathArray[k] = (char *)malloc(sizeof(char) * 250);

        strcpy(tempPath, path[k]);
        strcat(tempPath, "/");
        strcat(tempPath, args[0]);
	
        if (checkFileExist(tempPath)) { 
            strcpy(correctPathArray[i], tempPath);
            i++;
        }
    }
    return correctPathArray;
    
}

void checkFinished(){
	backgroundLists* iter = NULL;
        int status;
        int wait_pid=-1;
        for( iter = backgroundLRunning; iter!= NULL ;iter=iter->next){
        for(;(wait_pid=waitpid(-1, &status, WNOHANG|WUNTRACED|WCONTINUED))>0; wait_pid=-1){
	
    	//wait_pid=waitpid(-iter->pid, &(iter->status), WNOHANG|WUNTRACED|WCONTINUED);
    	
    	if(WIFEXITED(status) || WIFSTOPPED(status) ||  WSTOPSIG(status)){
    	//iter->status=1;
    	backgroundLists* temp = NULL;
        
    	for(temp = backgroundLRunning;temp!= NULL;temp=temp->next){
    	if(wait_pid==temp->pid)
    	runningToFinished(temp-> pid);
    	}
    	
    	}
    
    	    } 	}    }

void run(char** path, char* args[], int* background){
errorAddingRunningLL=0;
  /* int aha;
                       pid_t asa=waitpid(process_id,&aha,WNOHANG);
                        if(asa==-1){printf("error");}
                        else if(asa==0) printf("child calÄ±sÄ±yor lan mal");
                        else if(asa==process_id)printf("ohooo child bitti sen otur");*/
       
         
                 
                 
    //fork a child
    pid_t childPid;
    childPid = fork();

    //fork error
    if(childPid == -1) {
        fprintf(stderr, "Failed to fork.\n");
        return;
    }
    // Child part
    if(childPid == 0) {
   	signal(SIGINT, SIG_DFL);
        signal(SIGQUIT, SIG_DFL);
        signal(SIGTSTP, SIG_DFL);
        signal(SIGTTIN, SIG_DFL);
        signal(SIGTTOU, SIG_DFL);
        signal(SIGCHLD, SIG_DFL);
       
        execv(path[0], args);
        if(backgroundLRunning!=NULL){
        backgroundLists* temp = backgroundLRunning;
        if(temp->next==NULL) runningToFinished(temp-> pid);
        else if(temp->next->next == NULL) runningToFinished(temp->next-> pid);
        else {
        	for(; temp->next->next != NULL; temp=temp->next);
        	runningToFinished(temp-> pid);
        }}
        printf("Return not expected. There is an execv error\n");
        return;
    }
    // Parent part
    int status;

    // foreground
    if(*background == 0) { 
	checkCtrlZ();
        checkForegroundProcess = 1;
        currentFProcess = childPid;
        // wait until child terminates
        
     //   printf("%d",getppid());
        waitpid(childPid, &status, 0);
        checkForegroundProcess = 0;
    }

    // background
    else {
        
       
    	
        // adding background process and print
        char command[128]="";
        int i = 0;
        for(i=0; i < sizeArgs-1 ; i++){
        strcat(command,args[i]);
        strcat(command," ");
        command[strlen(command)+1]='\0';
        }
        addingBackground(childPid, command,backgroundLRunning);
        fflush(stdin);
        fflush(stdout);
        checkFinished();
        
        
        
    }	
    *background = 0;
   }
void print_ps_all(){
        fflush(stdin);
        fflush(stdout);
        checkFinished();
	backgroundLists* temp = backgroundLRunning;
	int i=0;
        printf("Running:\n");
        for(i = 0; temp != NULL; i++){
            printf("[%d] Pid: [%d] Command: %s\n", temp->order , temp->pid, temp->command);
            temp = temp->next;
        }
        backgroundLists* temp2 = backgroundLFinished;
        printf("Finished:\n");
        for(i = 0; temp2 != NULL; i++){
            printf("[%d] Pid: [%d] Command: %s\n", temp2->order , temp2->pid, temp2->command);
            temp2 = temp2->next;
        }
        backgroundLFinished=NULL;
}
 
int main(void)
{
            char inputBuffer[MAX_LINE]; /*buffer to hold command entered */
            int background; /* equals 1 if a command is followed by '&' */
            char *args[MAX_LINE/2 + 1]; /*command line arguments */
            char **paths;
    	    int status;
    	    
    	    // sigaction initialization
    	    struct sigaction signals;
   	    signals.sa_handler = signalHandler;
  	    //signalAction.sa_flags = SA_RESTART;
   	    // sigaction error check
   	    if ((sigemptyset( &signals.sa_mask ) == -1) || ( sigaction(SIGTSTP, &signals, NULL) == -1 ) ) {
        fprintf(stderr, "Couldn't set SIGTSTP handler\n");
            return 1;
    }	signal(SIGQUIT, SIG_IGN);
        signal(SIGTSTP, SIG_IGN);
        signal(SIGTTIN, SIG_IGN);
   
    
            while (1){
                        background = 0;
                        fflush(NULL);
                        sleep(1);
                        printf("myshell: ");
                        fflush(NULL);
                        /*setup() calls exit() when Control-D is entered */
                        
                        setup(inputBuffer, args, &background);


                      //  char* mergedArgs = (char*)malloc(sizeof(char)* 128);

        //concatenate the commands
        int count = 0;
        while (args[count] != NULL) {
               /* strcat(mergedArgs, args[count]);
                strcat(mergedArgs, " ");*/
                count++;
        }
                        
                       if (args[0] == NULL)
           	       continue;
                       else if(strcmp(args[0],"search")==0){
    				search(args);
			}
           		else if(strcmp(args[0],"bookmark")==0){
    				bookmarkCommand(args);
			}  
			else if(strcmp(args[0],"ps_all")==0){
    				args[count-1] = NULL;
    				print_ps_all();
			}
			else if(strcmp(args[0],"exit")==0){
			if(backgroundLRunning==NULL){
			exit(0);
			exit(1);
			}
			else{
			printf("There are still background process,you can not exit!\n");
			}
			
			}
           		 
                       else{                       
                        paths = fullPath(args);
               		 if(background == 1)
                  	  args[count-1] = NULL;
              		  run(paths, args, &background);
              		  }
                    //   checkFinished();
                       char cmd[50];

            }
}
