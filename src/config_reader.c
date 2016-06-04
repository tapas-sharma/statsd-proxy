/*******************************************************
 * Author  : Tapas Sharma
 * Email   : tapas.bits@gmail.com
 * Project : statsd_proxy
 * Clone   : https://tapassharma@bitbucket.org/tapassharma/statsd-proxy.git
 * Please read the README.md file before proceeding.
 *********************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <hiredis.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "conf_struct.h"
#include "log.h"

extern confStruct configuration;

/**
 * Initialize the configuration strucure with the default values
 * this will take the global configuration and bzero it out.
 * Then assign the default values.
 * @return
 * TRUE Always return TRUE since this is in memory DS
 */
ret_val init_configuration()
{
    //bzero the structure
    bzero(&configuration, sizeof(confStruct));
    //initialize the default values
    configuration.proxy_port = DEFAULT_PROXY_PORT;
    configuration.statsd_server_port = DEFAULT_STATSD_PORT;
    configuration.redis_server_port = DEFAULT_REDIS_PORT;
    //malloc the memory to store the IP in string format
    configuration.proxy_ip = (char *)malloc(IP_LEN);
    configuration.statsd_server_ip = (char *)malloc(IP_LEN);
    configuration.redis_server_ip = (char *)malloc(IP_LEN);
    //zero out the memory
    bzero(configuration.proxy_ip, IP_LEN);
    bzero(configuration.statsd_server_ip, IP_LEN);
    bzero(configuration.redis_server_ip, IP_LEN);
    //memcpy the default values in the malloc'd memory
    memcpy(configuration.proxy_ip, DEFAULT_PROXY_IP, strlen(DEFAULT_PROXY_IP) );
    memcpy(configuration.statsd_server_ip, DEFAULT_STATSD_IP, strlen(DEFAULT_STATSD_IP) );
    memcpy(configuration.redis_server_ip, DEFAULT_REDIS_IP, strlen(DEFAULT_REDIS_IP) );

    configuration.config_file = NULL;
    configuration.log_file = NULL;

    configuration.log_level = LOG_INFO; //default log level
  
    configuration.daemonize = FALSE; //DEFAULT value of daemonize, will change from the conf file
    configuration.default_init = TRUE; //TRUE since these are the default values
    configuration.log_file_closed = TRUE; //TRUE since we have not opened the file
    configuration.conf_file_closed = TRUE; //TRUE since we have not opened the file

    return TRUE;
}


/**
 * Prints the configuration at log level info
 */
void print_configuration()
{
    log(LOG_INFO, "configuration.proxy_port: %d\n\
      configuration.statsd_server_port: %d\n\
      configuration.redis_server_port: %d\n\
      configuration.proxy_ip: %s\n\
      configuration.statsd_server_ip: %s\n\
      configuration.redis_server_ip: %s\n\
      configuration.config_file: %p\n\
      configuration.log_file: %p\n\
      configuration.log_level: %d\n\
      configuration.daemonize: %d\n\
      configuration.default_init: %d\n\
      configuration.log_file_closed: %d\n\
      configuration.conf_file_closed: %d",
        configuration.proxy_port, 
        configuration.statsd_server_port, 
        configuration.redis_server_port,       
        configuration.proxy_ip,
        configuration.statsd_server_ip,
        configuration.redis_server_ip, 
        configuration.config_file?configuration.config_file:NULL,
        configuration.log_file?configuration.log_file:NULL,      
        configuration.log_level,  
        configuration.daemonize,
        configuration.default_init,
        configuration.log_file_closed,
        configuration.conf_file_closed);
}


/**
 * helper function to Get IP from the hostname
  * @return
 * TRUE When we were able to resolve the hostname
 * FALSE when there was some issue, look in the logs
 */
ret_val get_addr_info(char *hostname , char *ip)
{
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_in *h;
    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // use AF_INET6 to force IPv6
    hints.ai_socktype = SOCK_STREAM;

    if ( (rv = getaddrinfo( hostname , NULL , &hints , &servinfo)) != 0)
    {
        log(LOG_ERROR, "getaddrinfo: %s\n", gai_strerror(rv));
        return FALSE;
    }

    // loop through all the results and connect to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next)
    {
        h = (struct sockaddr_in *) p->ai_addr;
        strcpy(ip , inet_ntoa( h->sin_addr ) );
    }

    freeaddrinfo(servinfo); // all done with this structure
    return TRUE;
}


/**
 * Take the file path from the commandline and
 * open the file in the configuration structure
 * Read and assign the values.
 * Also, set the deafult_init flag to FALSE
 * @return
 * TRUE When we were able to set the configrations succesfully
 * FALSE when there was some issue, look in the logs
 */
ret_val read_config_file(const char *file_path)
{
    char iter, *line_buffer, property[24]={0} , ip[IP_LEN]={0};
    int char_cnt=0;
    configuration.config_file = fopen(file_path,"r"); //open the file in read_mode

    if(configuration.config_file == NULL)
    {
        log(LOG_INFO,"Cannot Open Configuration File %s.", file_path);
        return FALSE;
    }
    // start parsing the file char by char
    while( (iter=getc(configuration.config_file)) != EOF )
    {
        //skip newlines
        if(iter == '\n')
            continue;
        //skip comments
        if(iter=='#' || iter == '\n')
        {
            while( (iter=getc(configuration.config_file))!='\n' && iter != -1);
            continue;
        }
        char_cnt = 0;
        // go back one char since we would have moved ahead here
        fseek(configuration.config_file, -1 ,SEEK_CUR);
        while( (iter=getc(configuration.config_file))!=DELIM ) // read till DELIM
        {
            if( isspace(iter) > FALSE)
                continue;
            if(char_cnt > 24 || iter == DELIM)
                break;
            property[char_cnt] = iter;
            char_cnt++;
        }
        property[char_cnt]='\0';
        //skip spaces
        while( (iter=getc(configuration.config_file))  && iter != EOF)
        {
            if(isspace(iter))
                continue;
            else
                break;
        }
        fseek(configuration.config_file, -1 ,SEEK_CUR);      
        if(strcasecmp(property,"proxy_port") == 0)
        {
            fscanf(configuration.config_file,"%d",&configuration.proxy_port);
            log(LOG_INFO, "Proxy Port: %d",configuration.proxy_port );
        }
        else if(strcasecmp(property,"statsd_server_port") == 0)
        {
            fscanf(configuration.config_file,"%d",&configuration.statsd_server_port);
            log(LOG_INFO, "Statsd Server Port: %d",configuration.statsd_server_port );
        }
        else if(strcasecmp(property,"redis_server_port") == 0)
        {
            fscanf(configuration.config_file,"%d",&configuration.redis_server_port);
            log(LOG_INFO, "Redis Server Port: %d",configuration.redis_server_port );
        }
        else if( strcasecmp(property,"proxy_ip") == 0 ||
                 strcasecmp(property,"statsd_server_ip") == 0||
                 strcasecmp(property,"redis_server_ip")== 0
            )
        {
            char_cnt = 0;
            fpos_t temp;
            fgetpos(configuration.config_file, &temp);
            while( (iter=getc(configuration.config_file)) != '\n')
            {
                char_cnt++;
            }
            char_cnt++; //add new line too
            line_buffer=(char *)malloc(char_cnt+1);
            bzero(line_buffer, char_cnt+1);
            fsetpos(configuration.config_file, &temp);
            fgets(line_buffer, char_cnt, configuration.config_file);
            if ( get_addr_info(line_buffer, ip) == FALSE )
            {
                log(LOG_ERROR, "%s: Cannot resolve the IP for %s, using defaults", property, line_buffer);
                continue;
            }
            switch(property[0])
            {
            case 'p':
            case 'P':
                snprintf(configuration.proxy_ip, IP_LEN, "%s",ip);
                log(LOG_INFO, "Proxy IP: %s",configuration.proxy_ip );
                break;
	      
            case 's':
            case 'S':
                snprintf(configuration.statsd_server_ip, IP_LEN, "%s",ip);
                log(LOG_INFO, "Statsd IP: %s",configuration.statsd_server_ip );
                break;
	      
            case 'r':
            case 'R':
                snprintf(configuration.redis_server_ip, IP_LEN, "%s",ip);
                log(LOG_INFO, "Redis IP: %s",configuration.redis_server_ip );
                break;
            }
            free(line_buffer);
        }
        else if(strcasecmp(property,"log_level") == 0)
        {
            fscanf(configuration.config_file,"%d",&configuration.log_level);
            log(LOG_INFO, "Log Level: %s", LEVEL_TO_STRING(configuration.log_level) );
        }
        else if(strcasecmp(property,"daemonize") == 0)
        {
            int temp = 0;
            fscanf(configuration.config_file,"%d",&temp);
            if(temp > 0)
                configuration.daemonize=1;
            else
                configuration.daemonize=0;
            log(LOG_INFO, "Daemonize: %d",configuration.daemonize );
        }
        bzero(&property,24);
    }
    //close the file so no fd leaks
    fclose(configuration.config_file);
    configuration.config_file = NULL; // done reading
    configuration.default_init = FALSE;
    return TRUE;
}
