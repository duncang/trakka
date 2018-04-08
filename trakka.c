#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 


#include <my_global.h>
#include <mysql.h>

int quit = 0; 

void error(char *msg)
{
    perror(msg);
    exit(0);
}

/**
 *      \fn HandleSignal
 *      \brief Handle caught signals
 *      
 *      This function sets a flag (quit) to shut the program down.
 */
void HandleSignal(int signal) 
{ 
        fprintf(stdout,"Got Signal: %d\n",signal);
        quit = 1; 
}



int main(int argc, char *argv[])
{
    int sockfd, portno, n;


    struct sockaddr_in serv_addr;
    struct hostent *server;

    /* setup signal handlers */
    signal(SIGHUP,  HandleSignal);
    signal(SIGKILL, HandleSignal);
    signal(SIGTERM, HandleSignal);
    signal(SIGALRM, HandleSignal);
    signal(SIGINT, HandleSignal);



    char buffer[1024];
    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }
    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) 
    {
        error("ERROR connecting");
    }


    // print mysql info 
    printf("MySQL client version: %s\n", mysql_get_client_info());


    MYSQL *con = mysql_init(NULL);

    if (con == NULL) 
    {
      fprintf(stderr, "%s\n", mysql_error(con));
      exit(1);
    }

    if (mysql_real_connect(con, "localhost", "root", "tiger228", "trakka", 0, NULL, 0) == NULL) 
    {
      fprintf(stderr, "%s\n", mysql_error(con));
      mysql_close(con);
      exit(1);
    }  

    if (mysql_query(con, "SELECT * from icao_codes WHERE tail_num='VH-AAA'")) 
    {
      fprintf(stderr, "%s\n", mysql_error(con));
      mysql_close(con);
      exit(1);
    }


        

    while (quit == 0)
    {
        int i, got_message, message_type;

        bzero(buffer,sizeof(buffer));
        got_message = 0;

        do
        {
       
            n = read(sockfd,buffer,1);
            if (n < 0) 
            {
                error("ERROR reading from socket");
            }

            if (buffer[0] == 'M')
            {
                read(sockfd,buffer+1,1);

                if (buffer[1] == 'S')
                {
                    read(sockfd,buffer+2,1);

                    if (buffer[2] == 'G')
                    {
                        // got MSG header - continue reading buffer
                        // read until we get a newline char
                        i = 3;
                        do
                        {
                            read(sockfd, &buffer[i],1);
                            ++i;

                        } while (buffer[i-1] != '\n');

                        got_message = 1;
                        buffer[i+1] = '\0'; // just in case.. 
                        //printf("Got (%d) - message: %s\r\n", i, buffer);

                    } else
                    {
                        continue;
                    }

                } else
                {
                    continue;
                }
            } else
            {
                continue;
            }
        } while (got_message == 0);

        //printf("Got (%d) - message: %s\r\n", i, buffer);

        // message formats defined in http://woodair.net/sbs/Article/Barebones42_Socket_Data.htm 

        // now split the message and look for the fields we want
        message_type = atoi(&buffer[4]);
        switch(message_type)
        {
            case 1:
                break;
            case 2:
                break;
            case 3:

                // airborne position
                printf("got airborne position message: %s\n", buffer);

                int j, field;
                char field_data[30];
                bzero(field_data,sizeof(field_data));
                double ac_lat, ac_long;
                float ac_height;
                unsigned long icao_id;

                for (i=0,j=0,field=0;i<sizeof(buffer);i++)
                {
                    field_data[j++] = buffer[i];

                    if (buffer[i]==',')
                    {
                        if (field == 4)
                        {
                            // icao id
                            icao_id = strtol(field_data,NULL,16);
                        }

                        if (field == 11)
                        {
                            // height
                            ac_height = strtof(field_data,NULL);
                        }

                        if (field == 14)
                        {
                            // latitude
                            ac_lat = strtod(field_data,NULL);
                        }

                        if (field == 15)
                        {
                            // longitude
                            ac_long = strtod(field_data,NULL);
                        }

                        // start new field
                        field++;
                        j = 0;
                    //    continue;
                    }

                }

                printf("Decoded message_type (%d): id: %lx lat: %lf long:%lf height:%f\n", message_type, icao_id, ac_lat, ac_long, ac_height);


                /// INSERT INTO icao_codes (icao_id, tail_num) values (%s, %s)
                
                break;
            case 4:
                // airborne velocity 
                break;

            case 5:
                break;
            case 6:
                break;
            case 7:
                break;
            case 8:  
                break;
            default:

                fprintf(stderr,"Unknown message type %d\n", message_type);
                break;

        }


    }

    // 
    printf("Shuting down...\n");

    mysql_close(con);
    close(sockfd);

    return 0;
}

