#include <Bridge.h>
#include <Console.h>
#include <Process.h>

/**
 *	Arduino Yun Example
 *
 *	This example code is in the public domain.
 *	
 * 	@date 		July 3, 2013
 *  @author		Julio Terra
 *   
 */
 

struct Publisher {
  char *name;
  char *type;
  char *defaultValue;
  Publisher * next;
};

struct Subscriber{
  char *name;
  char *type;
  Subscriber * next;
};

Process brew;
String name;
String server;
int port;
String description;
Subscriber * subscribers;
Publisher * publishers;

// make pid removing process into an object that is declared and instantiated 
// in the set-up method so that these resources are relieved for later.
Process pids;
int const pidLength = 6;
int const sbPidsLen = 4;
char pid [6] = {'\0','\0','\0','\0','\0','\0'};
int sbPids [4] = {-1, -1, -1, -1};


void setup() { 
    delay(1000);

    Serial.begin(57600);
	while (!Serial) { 	Serial.println("connecting"); }
	Serial.println("App Started"); 

  	//Initialize Console and wait for port to open:
	Bridge.begin();
	Serial.println("Bridge Started"); 

	constructorSB();
	addPublish("please", "boolean");
	addPublish("work", "boolean");
	addSubscribe("it's", "boolean");
	addSubscribe("working", "boolean");
	connectSB(); 
   
//	Console.buffer(64);
//	Serial.println("Console Started"); 
} 

void constructorSB() {
	name = "Arduino Yun Revised";
	server = "sandbox.spacebrew.cc";
	port = 9000;
	description = "no description";
}

void addPublish(const String name, const String type) {
	Publisher *p = new Publisher();
	p->name = createString(name.length() + 1);
	p->type = createString(type.length() + 1);
	name.toCharArray(p->name, name.length() + 1);
	type.toCharArray(p->type, type.length() + 1);

	if (publishers == NULL){
		publishers = p;
	} else {
		Publisher *curr = publishers;
		while(curr->next != NULL){
			curr = curr->next;
		}
		curr->next = p;
	}
}

void addPublish(char * name, char * type) {
	addPublish( String(name), String(type) );
}

void addSubscribe(const String name, const String type) {
	// subscriber += name + "," + type + ";";
	Subscriber *p = new Subscriber();
	p->name = createString(name.length() + 1);
	p->type = createString(type.length() + 1);
	name.toCharArray(p->name, name.length() + 1);
	type.toCharArray(p->type, type.length() + 1);

	if (subscribers == NULL){
		subscribers = p;
	} else {
		Subscriber *curr = subscribers;
		while(curr->next != NULL){
			curr = curr->next;
		}
		curr->next = p;
	}
}

void addSubscribe(char * name, char * type) {
	addSubscribe( String(name), String(type) );
}

void connectSB() {

	killBrewPids();

 	brew.begin("python"); // Process should launch the "curl" command
	brew.addParameter("/usr/lib/python2.7/spacebrew.py"); // Process should launch the "curl" command
	brew.addParameter("--server");
	brew.addParameter(server);
	brew.addParameter("--port");
	brew.addParameter(String(port));
	brew.addParameter("-n");
	brew.addParameter(name);
	brew.addParameter("-d");
	brew.addParameter(description);

	if (subscribers != NULL) {
		Subscriber *curr = subscribers;
		while(curr != NULL){
			Serial.print("sub name: ");
			Serial.println(curr->name);

			brew.addParameter("-s"); // Add the URL parameter to "curl"
			brew.addParameter(curr->name); // Add the URL parameter to "curl"
			brew.addParameter(","); // Add the URL parameter to "curl"
			brew.addParameter(curr->type); // Add the URL parameter to "curl"

			if (curr->next == NULL) curr = NULL;
			else curr = curr->next;
		}
	}
	if (publishers != NULL) {
		Publisher *curr = publishers;
		while(curr != NULL){
			Serial.print("pub name: ");
			Serial.println(curr->name);

			brew.addParameter("-p"); // Add the URL parameter to "curl"
			brew.addParameter(curr->name); // Add the URL parameter to "curl"
			brew.addParameter(","); // Add the URL parameter to "curl"
			brew.addParameter(curr->type); // Add the URL parameter to "curl"

			if (curr->next == NULL) curr = NULL;
			else curr = curr->next;
		}
	}

    Serial.println("connectSB - starting console");

	Console.begin();
	delay(500);

	brew.run();                   // Run the process and wait for its termination	

	while (!Console) { ; }        

    Serial.println("connectSB - running spacebrew.py script");
}


void loop() { 
} 


/**
 * method that gets the pid of all spacebrew.py instances running on the linino.
 */
void getSbPid() {

	// request the pid of all python processes
	pids.begin("python");
	pids.addParameter("/usr/lib/python2.7/getSbPid.py"); // Process should launch the "curl" command
	pids.run();

	Serial.println("getSbPid - process running");

	int sbPidsIndex = 0;
    int pidCharIndex = 0;
    char c = '\0';

	while ( pids.available() > 0 ) {

	    c = pids.read();

		if ( c >= '0' && c <= '9' ) {
			pid[pidCharIndex] = c;
			pidCharIndex = (pidCharIndex + 1) % pidLength;
		} 

		else if ( (c == ' ' || c == '\n') && pidCharIndex > 0) {
			sbPids[sbPidsIndex] = atoi(pid);
			if ( sbPidsIndex < (sbPidsLen - 1) ) sbPidsIndex = (sbPidsIndex + 1);    		

			for( int i = 0; i < pidLength; i++ ){ 
				pid[i] = '\0';
				pidCharIndex = 0;
			}
		}
	}

	// print out the pid of all python processes
	Serial.println("\nSB pids recap: ");
	for (int i = 0; i < sbPidsIndex; i++) {
		Serial.print(i);
		Serial.print(" : ");
		Serial.println(sbPids[i]);                
	}
}

/**
 * method that kills all of the spacebrew.py instances that are running 
 * on the linino.
 */
void killBrewPids() {
	getSbPid();
	delay(400);

	for (int i = 0; i < sbPidsLen; i ++) {
		if (sbPids[i] > 0) {
			char * newPID = itoa(sbPids[i], pid, 10);
			Serial.print("deleting pid: ");
			Serial.println(newPID);

			Process p;
			p.begin("kill");
			p.addParameter("-9");
			p.addParameter(newPID);		// Process should launch the "curl" command
			p.run();            		// Run the process and wait for its termination	

			delay(400);						
		}
	}
}

static char *cloneString(char *s){
	int n = strlen(s);
	char *out = (char *)malloc(n+1);//new char[n];
	strcpy(out, s);
	return out;
}

static char *createString(int n){
	char *out = (char *)malloc(n+1);//new char[n];
	return out;
}

