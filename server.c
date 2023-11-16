#include <netinet/in.h>
#include <stddef.h>
#include<stdio.h>
#include <sys/socket.h>
#include <sys/types.h> // for unsigned long type
#include <stdlib.h>// for exit()
#include <unistd.h> //for read()
#include <string.h>
#include <time.h>
#define buff_size 300000
char* ROOT_DIR ="htdocs";

//struct for server
struct Server{
    int domain;
    int service;
    int protocol;
    u_long interface;
    int port;
    int backlog;//provides a hint to the implementation which the implementation shall use to limit the number of outstanding connections in the socket's listen queue.

    struct sockaddr_in address;
    /*
    The sockaddr_in structure is used to represent addresses in the 
    Internet address family (AF_INET). The sockaddr_in structure contains the following members:
    
    structure od sockaddr_in{
        sin_family: The address family.
        sin_port: The port number.
        sin_addr: The IP address.
    }

    */
    int socket;//socket
    void (*launch_server)(struct Server *server);//function pointer for the server launching function which has a pointer to a Server stucture as parameter


};

//server constructor
struct Server server_constructor(int domain,int service,int protocol,u_long interface,int port,int backlog, void (*launch_server)(struct Server *server)){
    struct Server server;

    server.domain=domain;
    server.service=service;
    server.protocol=protocol;
    server.interface=interface;
    server.port=port;
    server.backlog=backlog;
    server.launch_server=launch_server;


    server.address.sin_family=domain;
    server.address.sin_port=htons(port);//h to network -> convert int port to bytes that refer to port
    server.address.sin_addr.s_addr=htonl(interface);

    //creating socket connection to the network
    //gives address the where socket is
    server.socket=socket(domain,service,protocol);
    if(server.socket<0){
        perror("Failure while creating socket!\n");
        exit(1);
    }


    //bind-associates the socket with a local address (ip address and a port number)
    if(bind(server.socket,(struct sockaddr *)&server.address,sizeof(server.address))<0){
        perror("Failure while binding to socket!\n");
        exit(1);
    }


    //listening to server
    if(listen(server.socket,server.backlog)<0){
        perror("Failure to start listening!\n");
        exit(1);
    }


    return server;
}

//decoding the path to remove hex values like %20 for spaces and make it the valid path
char* decode(char *url){
    size_t buffLen=strlen(url);

    //allocate a memory for the decode length 
    char* decoded=malloc(buffLen+1);
    size_t decodedLen=0;

    for(size_t i=0;i<buffLen;i++){
        //when found escape character '%'
        if(url[i]=='%'){

            //when there are two characters after escape character 
            if(i+2<buffLen){
                int hexadecimal;
                //converting two characters after '%' from hexadecimal to decimal(ascii)
                sscanf(url+i+1,"%2x",&hexadecimal);

                //assigning that ascii value to the new decoded buff
                decoded[decodedLen++]=hexadecimal;
                i+=2;//to skip read characters
            }else{
                decoded[decodedLen++]=url[i];
            }
        }else{
            decoded[decodedLen++]=url[i];
        }
    }

    //adding end character
    decoded[decodedLen]='\0';

    return decoded;
}





char* getFile(char *request,size_t* file_size,char* extenstionPass){
    char *request_type,*url,*request_body,*params,*content;
    params=NULL;
    request_body=NULL;
    FILE *file;
    //since only non get req have body
    request_body = strstr(request, "\r\n\r\n")+4; // Read the remaining content
    // printf("req bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb- %s\n",request_body);
    if (request_body != NULL && strlen(request_body) > 0) {
        printf("Body: %s\n", request_body);
    }
    
    //parsing the req
    // Parse request line
    char *request_line = strtok(request, "\n");
    
    request_type = strtok(request_line, " ");


    url = strtok(NULL, " ");

    // Parse URL and parameters
    char *question_mark_pos = strchr(url, '?');
    if (question_mark_pos != NULL) {
        // URL contains parameters
        *question_mark_pos = '\0'; // Split URL and parameters
        params = question_mark_pos + 1;
        printf("Request Type: %s\n", request_type);
        printf("URL: %s\n", url);
        printf("Parameters: %s\n", params);
    } else {
        printf("Request Type: %s\n", request_type);
        printf("URL: %s\n", url);
    }
    //ignoring favicon.ico req.....................................................................................................................................
    if(!strcmp(url,"/favicon.ico")){
        printf("Ignored favicon.ico request\n\n");
        return NULL;
    }


    //decoding url to get finalized path
    url=decode(url);
    //finalized path store correct decode file path +100 for more if necessary
    char *finalizedPath=malloc(strlen(ROOT_DIR)+strlen(url)+100);
    strcpy(finalizedPath,ROOT_DIR);
    printf("fpath : %s\n",finalizedPath);
    strcat(finalizedPath, url);
    //debugging
    /*
    printf("decode url: %s\n",url);
    printf("Request Type: %s\n", request_type);
    printf("URL: %s\n", url);
    printf("Parameters: %s\n", params);
    */
    char *extension = strrchr(finalizedPath, '.');
    if (extension == NULL) {//no extentions -> it must be a directory
        printf("%s has no extension. Thus a directory\n", finalizedPath);
        
        // printf("%c %d\n",path[-1],strcmp(path,ROOT_DIR));
        if(finalizedPath[strlen(finalizedPath)-1]!='/'  && strcmp(finalizedPath,ROOT_DIR))strcat(finalizedPath,"/");//unless its root path and dont have a'/' at end
        

        // Check if the res index.php or index.html file exists
        char *temp=malloc(10000);
        strcpy(temp, finalizedPath);
        strcat(temp, "index.php");
        if (access(temp, F_OK) != -1) {//when there is an index.php
            printf("The file %s exists.\n", temp);
            extension=".php";
            strcat(finalizedPath, "index.php");
        } else {
            temp[0]='\0';
            strcpy(temp, finalizedPath);
            strcat(temp, "index.html"); 
            if (access(temp, F_OK) != -1) {//when there is an index.html
                printf("The file %s exists.\n", temp);
                extension=".html";
                strcat(finalizedPath, "index.html");
            }else{//when there is no index.php or index.html- 404
                printf("No index file in the directory!\n");
                return NULL;
                ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            }
        }
        free(temp);
    }
    printf("Finalized path- %s\n\n",finalizedPath);
    printf("extension- %s\n\n",extension);

    //copying extension to pass 
    if(extension!=NULL){
        strcpy(extenstionPass,extension);
    }

    if (access(finalizedPath, F_OK) != -1) {//when the req file exists
        printf("The file %s exists.\n", finalizedPath);

        //openning file for reading
        file = fopen(finalizedPath, "rb"); 

        if (file == NULL) {
            printf("Error opening file : %s\n\n",finalizedPath);
            exit(1);
        }
        printf("dde\n");

        // Get the file size
        fseek(file, 0, SEEK_END);
        *file_size = ftell(file);
        rewind(file);
        printf("dde\n");

        content = (char *)malloc(*file_size + 1); // +1 for null terminator
        printf("File size:%d\n",(int)*file_size);
        if (content == NULL) {
            printf("dde\n");
            perror("Error allocating memory");
            fclose(file);
            exit(1);
        }

        // Read the entire file into the buffer
        if (fread(content, 1, *file_size, file) != *file_size) {
            perror("Error reading file");
            fclose(file);
            free(content);
            exit(1);
        }
        
        // Null-terminate the buffer
        content[*file_size] = '\0';

        // Close the file
        fclose(file);
        
        
        //if file is a php, copy the content to a file with the params and execute data
        if(!strcmp(extension, ".php")){
            FILE *file;

            // Open the file for writing (overwriting)
            file = fopen("temp.php", "w");

            // Check if the file was successfully opened
            if (file == NULL) {
                printf("Error opening the file.\n");
                exit(1);
            }
            printf("fffffffffffffsssssssssssssss%s\n",params);
            
            //check if this is a post method
            if(!strcmp(request_type, "POST")){
                if (request_body != NULL && strlen(request_body) > 0) {
                    char* token = strtok(request_body, "&");
                    char* keyValue;
                    char* key;
                    char* value;
                    
                    // Create an array to store key-value pairs
                    char* keyValuePairs[10];
                    int count = 0;

                    while (token != NULL) {
                        keyValuePairs[count] = token;
                        count++;
                        token = strtok(NULL, "&");
                    }
                    // Create a PHP-like associative array

                    fprintf(file, "<?php\n$_POST=array(");
                    // printf("Array (\n");
                    for (int i = 0; i < count; i++) {
                        keyValue = keyValuePairs[i];
                        key = strtok(keyValue, "=");
                        value = strtok(NULL, "=");

                        char* decodedValue = decode(value);
                        fprintf(file, "'%s'=>'%s',",key,decodedValue);  
                        free(decodedValue);
                    }
                    fprintf(file, ");?>");
                }else if(params!=NULL){
                    char* token = strtok(params, "&");
                    char* keyValue;
                    char* key;
                    char* value;
                    
                    // Create an array to store key-value pairs
                    char* keyValuePairs[10];
                    int count = 0;

                    while (token != NULL) {
                        keyValuePairs[count] = token;
                        count++;
                        token = strtok(NULL, "&");
                    }
                    // Create a PHP-like associative array

                    fprintf(file, "<?php\n$_POST=array(");
                    // printf("Array (\n");
                    for (int i = 0; i < count; i++) {
                        keyValue = keyValuePairs[i];
                        key = strtok(keyValue, "=");
                        value = strtok(NULL, "=");
                        if(key==NULL||value==NULL)break;

                        char* decodedValue = decode(value);
                        fprintf(file, "'%s'=>'%s',",key,decodedValue);  
                        free(decodedValue);
                    }
                    fprintf(file, ");?>");
                }
            }else if(!strcmp(request_type, "GET")&& params!=NULL){

                    char* token = strtok(params, "&");
                    char* keyValue;
                    char* key;
                    char* value;
                    
                    // Create an array to store key-value pairs
                    char* keyValuePairs[10];
                    int count = 0;

                    while (token != NULL) {
                        keyValuePairs[count] = token;
                        count++;
                        token = strtok(NULL, "&");
                    }
                    // Create a PHP-like associative array

                    fprintf(file, "<?php\n$_GET=array(");
                    printf("Array (\n");
                    for (int i = 0; i < count; i++) {
                        
                        keyValue = keyValuePairs[i];
                        key = strtok(keyValue, "=");
                        value = strtok(NULL, "=");
                        if(key==NULL||value==NULL)break;

                        // printf("d\n");
                        char* decodedValue = decode(value);
                        fprintf(file, "'%s'=>'%s',",key,decodedValue);  
                        free(decodedValue);
                        
                    }
                    fprintf(file, ");?>");
            }
            printf("fffffffffffffsssssssssssssss\n");
            
            // Write content to the file
            fprintf(file, "%s",content);

            // Close the file
            fclose(file);

            //executing file
            free(content);
            

            // Open a pipe to the command and execute it
            FILE *pipe = popen("php temp.php", "r");
            printf("pipe ran for php temp.php");
            if (pipe == NULL) {
                perror("popen");
                exit(1);
            }

            char *output = NULL;
            *file_size = 0;
            char buffer[128];

            // Read the output of the PHP script and append it to the buffer
            while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
                size_t buffer_len = strlen(buffer);
                output = (char *)realloc(output, *file_size + buffer_len + 1);
                if (output == NULL) {
                    perror("realloc");
                    exit(1);
                }
                strcpy(output + *file_size, buffer);
                *file_size += buffer_len;
            }

            // Close the pipe
            pclose(pipe);
            printf("about to return output\n");
            return output;

        }else return content;//if not a php, just return data


    }else{//when the file doesnt exist

        printf("No such file in the directory!\n");
        return NULL;
    }

}

//launching server
void launch_server(struct Server *server){
    while(1){
        char *response=malloc(buff_size);
        char *buffer=malloc(buff_size);
        char *extensionPass=malloc(32);
        size_t *file_size=malloc(100);
        printf("****Listening****\n");
        int add_len=sizeof(server->address);//length of address
        struct sockaddr *sockAddPtr= (struct sockaddr *)&server->address;//casting the address to sockaddr pointer

        //accepting socket
        int newSocket=accept(server->socket,sockAddPtr,(socklen_t *)&add_len);
        
        //read() uses to read data from file or socket
        read(newSocket,buffer,buff_size);
        // printf("%s\n",buffer);
        char *content=getFile(buffer,file_size,extensionPass);

        // Get the current time
        time_t now;
        struct tm *tm_info;
        char formatted_time[32]; // A buffer to hold the formatted date and time

        time(&now);
        tm_info = gmtime(&now); // Use gmtime to get UTC time

        // Format the date and time
        strftime(formatted_time, sizeof(formatted_time), "%a, %d %b %Y %H:%M:%S GMT", tm_info);

        if(content==NULL){//null will be passed if no file found-404
            printf("No content delivered to send !\n");
        
            snprintf(response, buff_size, "HTTP/1.1 404 Not Found\nServer: Chathura/1.0\nDate: %s\nContent-Type: text/html\nContent-Length: \n\n<!DOCTYPE html><html><head><title>404 Not Found</title></head><body><h1>404 Not Found</h1><p>The requested resource could not be found on this server.</p><img src='https://http.cat/images/404.jpg'/></body></html>",formatted_time);

        }else if(extensionPass!=NULL && !strcmp(extensionPass, ".css")){//for css
            snprintf(response, buff_size, "HTTP/1.1 200 OK\nDate: %s\nServer: Chathura/1.0 (Win32) \nContent-Length:%zu \nContent-Type: text/css\nConnection: Closed\n\n", formatted_time,*file_size);
            strcat(response, content);
        }else {//for php and html
            snprintf(response, buff_size, "HTTP/1.1 200 OK\nDate: %s\nServer: Chathura/1.0 (Win32) \nContent-Length:%zu \nContent-Type: text/html\nConnection: Closed\n\n", formatted_time,*file_size);
            strcat(response, content);
            
        }
        // printf("\n\n response: \n%s\n\n",response);

        write(newSocket,response,strlen(response));
        
        close(newSocket);
        if(response!=NULL)free(response);
        if(content!=NULL)free(content);
        if(buffer!=NULL)free(buffer);
        if(file_size!=NULL)free(file_size);
    }
    
}

int main(){

    /*
    domain - AF_INET - internet domain
    service(type) - SOCK_STREM - This type of socket is used for applications such as web servers and file transfer protocols. 
    protocol - 0 -> Specifying a protocol of 0 causes socket() to use an unspecified default protocol appropriate for the requested socket type.
    interface - INADDR_ANY -> INADDR_ANY is a special IP address that is used when you don't want to bind a socket to a specific IP address. When you use this value as the address when calling bind(), the socket accepts connections to all the IPs of the machine.
    port- 80
    backlog - 10
    (*launch)()
    */
    struct Server server=server_constructor(AF_INET, SOCK_STREAM, 0, INADDR_ANY, 2728, 10, launch_server);
    server.launch_server(&server);
}