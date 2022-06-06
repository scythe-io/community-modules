/*++

 Copyright (c) SCYTHE, Inc. Use is subject to agreement.

 --*/
#define _GNU_SOURCE
#include "common.h"
#include <stdio.h>
#include <libssh/libssh.h>
#include <libssh/sftp.h>
#include <fcntl.h>
#include "nxjson.h"
#include <sys/stat.h>

PSCYTHE_CLIENT g_pClient = NULL;

// {6d139160-b684-11ec-9b21-8b2a13dd1ff5}
static const GUID sshModuleId =
{ 0x6d139160, 0xb684, 0x11ec, { 0x9b, 0x21, 0x8b, 0x2a, 0x13, 0xdd, 0x1f, 0xf5}};

static const SCYTHE_MODULE_TYPE sshModuleType = CATEGORY_WORKER;


struct ssh_sesh {
    ssh_session session;
    ssh_channel channel; 
    ssh_scp scp;
    sftp_session sftp;
    char *status; 
    int success;
} ssh_sesh;

struct creds {
    const char *host;
    const char *username;
    const char *password;
    int port;
    const char *command; 
    const char *pkey;
    const char *filename;
    const char *filebytes;
    const char *filepath;
    const char *dirpath;
    int is_vfs;
    int pkey_vfs;
};

void free_channel(ssh_channel channel);
void free_session(ssh_session session);
void free_scp(ssh_scp scp);
void error(ssh_session session);
void parseJSON(char *request, struct creds *);
char* run_command(struct ssh_sesh *,const char *);
void init_session(struct creds *,struct ssh_sesh*);
void init_channel(struct ssh_sesh *);
void init_scp(struct ssh_sesh *, const char *);
void free_sftp(sftp_session sftp);
void init_sftp(struct ssh_sesh *);


char * readFile(const char *);
//void scp_read(struct ssh_sess *);
void scp_write_file(struct ssh_sesh *,const char *file,const char *filename);

void free_channel(ssh_channel channel){
    ssh_channel_send_eof(channel);
    ssh_channel_close(channel);
    ssh_channel_free(channel);
}

void free_session(ssh_session session){
    ssh_disconnect(session);
    ssh_free(session);
}

void free_scp(ssh_scp scp){
    ssh_scp_close(scp);
    ssh_scp_free(scp);
}

void free_sftp(sftp_session sftp){
    sftp_free(sftp);
}

void error(ssh_session session){
    free_session(session);
}

void init_session(struct creds *cred, struct ssh_sesh *sesh) {
    int rc = 0;
    sesh->success = 0; // 1= true, 0 = false;

    sesh->session = ssh_new();
    if (sesh->session == NULL) {
        fprintf(stderr, "Error: condition Session Not Initialized\n");
        sesh->status = "Error: Session Not Initialized";
    }
    
    ssh_options_set(sesh->session, SSH_OPTIONS_HOST,cred->host);
    ssh_options_set(sesh->session, SSH_OPTIONS_PORT, &cred->port);
    ssh_options_set(sesh->session, SSH_OPTIONS_USER,cred->username);

    rc = ssh_connect(sesh->session); 
    fprintf(stderr, "%d\n", rc);
    if (rc != SSH_OK){
        sesh->status = "Error: Session Not Initialized\n";
        error(sesh->session);
    }
    // do auth 
    // Start with pKEY if provided; 
    if (cred->pkey != NX_JSON_NULL && cred->pkey != NULL) {
        if (cred->pkey_vfs != 0 && cred->pkey_vfs != NX_JSON_NULL) {
            // ssh_key key = NULL;
            // Do ssh stuff
        }
    }// end auth wit pkey
    if (cred->password != NX_JSON_NULL && cred->password != NULL ){
        rc = ssh_userauth_password(sesh->session,NULL, cred->password);
    }
    // check if auth is successful 
    switch (rc){
        case SSH_AUTH_ERROR:
            asprintf(&sesh->status,"Error: SSH_AUTH_ERROR: (%s,%s) on %s:%d\n", cred->username, cred->password, cred->host, cred->port); 
            free_session(sesh->session);
            break;
        case SSH_AUTH_DENIED:
            asprintf(&sesh->status,"Error: SSH_AUTH_DENIED: (%s,%s) on %s:%d\n", cred->username, cred->password, cred->host, cred->port);
            free_session(sesh->session);
            break;
        case SSH_AUTH_PARTIAL:
            asprintf(&sesh->status,"Error: SSH_AUTH_PARTIAL: (%s,%s) on %s:%d\n", cred->username, cred->password, cred->host, cred->port);
            free_session(sesh->session);
            break;
        case SSH_AUTH_SUCCESS:
            sesh->success = 1;
            asprintf(&sesh->status,"SSH_AUTH_SUCCESS: (%s,%s) on %s:%d\n", cred->username, cred->password, cred->host, cred->port);
            break;
        case SSH_AUTH_AGAIN:
            asprintf(&sesh->status,"Error: SSH_AUTH_AGAIN: (%s,%s) on %s:%d\n", cred->username, cred->password, cred->host, cred->port);
            free_session(sesh->session);
            break;
        default:
            asprintf(&sesh->status,"Unexpected Error: (%s,%s) on %s:%d\n", cred->username, cred->password, cred->host, cred->port);
            free_session(sesh->session);
            break;
    }
}//end 

/*
 * Create channel to send commands or scp files through 
 * This is only needed if trying to run commands on remote machine
 * For password spraying init_sessino is enough to test. 
*/ 
void init_channel(struct ssh_sesh *sesh){
    int rc = 0;
    sesh->success = 0;
    if (sesh->session == NULL){
        //perror("Error: Session Not Initialized\n");
        sesh->status = "Error: Session not Initialized\n";
        return;
    }//end 
    sesh->channel = ssh_channel_new(sesh->session);
    if (sesh->channel == NULL){
        //perror("Error: Channel Not Initialized\n");
        sesh->status = "Error: Channel not Initialized\n";
        return;
    }
    rc = ssh_channel_open_session(sesh->channel);
    if (rc != SSH_OK){
        ssh_channel_free(sesh->channel);
        //perror("Error: Channel Open Session Failed;");
        sesh->status = "Error: Channel Open Session Failed\n";
        return;
    }
    sesh->success = 1;
    sesh->status = "Channel Successfully Created!\n";
    return;
}

char* run_command(struct ssh_sesh *sesh,const char *cmd) {
    sesh->success = 0;
    char buffer[1024];
    char *resp;
    int rc = 0;
    int nbytes;
    rc = ssh_channel_request_exec(sesh->channel, cmd);
    if ( rc != SSH_OK){
        free_channel(sesh->channel);
        sesh->status = "Error: SSH Channel Request Exec Failed.\n";
        return NULL;
    }//end
    while (nbytes > 0) {
        nbytes = ssh_channel_read(sesh->channel, buffer, sizeof(buffer),0);
        if (fwrite(buffer, 1, nbytes, stdout) != nbytes) {
            free_channel(sesh->channel);
            sesh->status = "Error: Reading bytes from remote buffer\n";
            return NULL;
        }//endd
        nbytes = ssh_channel_read(sesh->channel, buffer, sizeof(buffer),0);
        asprintf(&resp,"%s\n",buffer);
        //perror(buffer);
    }
    if (nbytes < 0){
        free_channel(sesh->channel);
        sesh->status = "Error: Writing to Buffer Failed\n";
        return NULL;
    }
    sesh->success = 1;
    sesh->status = "SSH Buffer Written!";
    return resp;
}

void init_sftp(struct ssh_sesh *sesh){
    sesh->success = 0;
    sesh->sftp = sftp_new(sesh->session);
    int rc;
    if (sesh->sftp == NULL){
        asprintf(&sesh->status,"Error: Allocating SFTP session %s\n",ssh_get_error(sesh->session));
        return;
    }
    rc = sftp_init(sesh->sftp);
    if (rc != SSH_OK){
        asprintf(&sesh->status,"Error: initializing SFTP session: code %d\n",sftp_get_error(sesh->sftp));
        return;
    }//end 
    sesh->success = 1;
    asprintf(&sesh->status,"Success: SFTP Session Initialized\n");
}

void init_scp(struct ssh_sesh *sesh,const char *path){
    int rc; // Response Code
    sesh->success = 0;
    //perror(path);
    sesh->scp = ssh_scp_new(sesh->session, SSH_SCP_WRITE | SSH_SCP_RECURSIVE , path); // the . same as running scp -r for SSCH_SCP_RECURSIVE
    if (sesh->scp == NULL){
        asprintf(&sesh->status, "Error: Creating a new SCP Session. %s\n", ssh_get_error(sesh->session));
        //perror(sesh->status);
        return;
    }
    rc = ssh_scp_init(sesh->scp);
    if ( rc != SSH_OK){
        asprintf(&sesh->status,"Error: initializing scp session: %s\n", ssh_get_error(sesh->session));
        //perror(sesh->status);
        return;
    }
    sesh->success = 1;
    sesh->status = "SCP Initialized\n";
    //perror(sesh->status);
    return;
}

void scp_write_file(struct ssh_sesh *sesh,const char *file,const char *filename){
    int rc;
    sesh->success = 0;
    
    int length = strlen(file);

    rc = ssh_scp_push_file(sesh->scp,filename,strlen(file), S_IRUSR | S_IWUSR | O_CREAT );
    if (rc != SSH_OK){
        asprintf(&sesh->status,"Error: Can't open remote file: %s\n", ssh_get_error(sesh->session));
        free_scp(sesh->scp);
        return;
    }//end 

    rc = ssh_scp_write(sesh->scp, file, length);
    if (rc != SSH_OK){
        asprintf(&sesh->status, "Error: Can't write to remote file: %s\n",ssh_get_error(sesh->session));
        free_scp(sesh->scp);
        return;
    }
    asprintf(&sesh->status,"File Bytes (%d) written to %s.",length, filename);
    sesh->success = 1;
}

void transfer_file_sftp(struct ssh_sesh *sesh, const char *filebytes, const char *filename){
    int access_type = O_WRONLY | O_CREAT | O_TRUNC;
    int length = strlen(filebytes);
    int rc, nwritten;
    sesh->success = 0;
    sftp_file file; 
    
    file = sftp_open(sesh->sftp,filename, access_type, S_IRWXU);
    if (file == NULL){
        asprintf(&sesh->status,"Can't open file for writing: %s\n",ssh_get_error(sesh->session));
    }
    nwritten = sftp_write(file, filebytes, length);
    if (nwritten != length){
        asprintf(&sesh->status, "Can't write data to file: %s\n",ssh_get_error(sesh->session));
        sftp_close(file);
    }

    rc = sftp_close(file);
    if (rc != SSH_OK){
        asprintf(&sesh->status,"Can't close the written file: %s\n", ssh_get_error(sesh->session));
    }//end
    sesh->status = "File written. ";

    sesh->success = 1;

}

/*
void scp_read(struct ssh_sesh *sesh, char *filepath){
    int rc;
    sesh->success = 0;
    sesh->scp = ssh_scp_new(sesh->session, SSH_SCP_READ, *filepath);

    if (scp == NULL){
        asprintf(&sesh->status,"Error: allocating scp session: %s\n",ssh_get_error(sesh->session));
        return;
    }//end
    
    rc = ssh_+scp_init(scp);
    if (rc != SSH_OK){
        asprintf(&sesh->status,"Error inilizations scp session: %s\n",ssh_get_error(Sesh->session));
        ssh_scp_free(scp);
    }//end

}
*/

INT init(PSCYTHE_CLIENT pClient, PVOID pvReserved)
{
    g_pClient = pClient;
        
    return SCYTHE_NOERROR;
}

ERROR_T run(PSCYTHE_MESSAGE pMessage, PVOID pvReserved)
{
    ERROR_T status = SCYTHE_NOERROR;
    char *resp; 
    char *request = (char*) pMessage->Message;
    struct creds cred;
    
    parseJSON(request, &cred);
    struct ssh_sesh sesh;
    init_session(&cred, &sesh);
    
    if (sesh.success == 0){
        fprintf(stderr,"%s",sesh.status);
    }
    resp = sesh.status;
    init_channel(&sesh);

    if (cred.filename != NULL){
        init_sftp(&sesh);
        char *remote_filepath;
        asprintf(&remote_filepath,"%s/%s",cred.dirpath,cred.filename);
        if (cred.is_vfs == 0) {
            char *buffer;
            buffer = readFile(cred.filepath);
            transfer_file_sftp(&sesh,buffer,remote_filepath);
        }//end if
        if (cred.is_vfs == 1){
            scp_write_file(&sesh,cred.filebytes,remote_filepath);
        }//end if
    }//end if 
    if (cred.command != NULL){
        init_channel(&sesh);
        if (sesh.success == 1){
            char *data;
            data = run_command(&sesh,cred.command);
            if (sesh.success == 1){ 
                asprintf(&resp,"Remote Host: (%s:%d)\nCommand Run: %s\n: Output:%s\n", cred.host, cred.port, cred.command, data);
            }//end
        }//end 
    }//end
   free_sftp(sesh.sftp);
   free_channel(sesh.channel);
   free_session(sesh.session);
    status = g_pClient->PostToMessagingSubsystem(pMessage->MessageId, 0, 0,
        CATEGORY_OUTPUT_COMMUNICATION,
        &sshModuleId,
        NULL,
        strlen(resp), (PBYTE)resp);

    return status;
}

ERROR_T getinfo(PSCYTHE_MODULE_INFORMATION pInformation)
{
    ERROR_T status = SCYTHE_ERROR_INVALID_PARAMETER;

    if (NULL != pInformation)
    {
        if (pInformation->cbSize >= sizeof(SCYTHE_MODULE_INFORMATION))
        {
            pInformation->ModuleVersion.wMajor = 1;
            pInformation->ModuleVersion.wMinor = 0;
            pInformation->ModuleType = sshModuleType;
            memcpy(&(pInformation->ModuleId), &sshModuleId, sizeof(GUID));
            status = SCYTHE_NOERROR;
        }
    }

    return status;
}

// Required function: deinit
ERROR_T deinit(PVOID pvReserved)
{
    return SCYTHE_NOERROR;
}


char * readFile(const char * filename){
    FILE *fp = fopen(filename, "rb");
    if (fp == NULL){
        return "Error with file pointer";
    }//end
    fseek(fp,0L,SEEK_END);
    long size = ftell(fp) +1;
    fclose(fp);
    fp = fopen(filename,"rb");
    void* content = memset(malloc(size), '\0',size);
    fread(content, 1, size-1, fp);
    fclose(fp);
    return (char* ) content;
}

/*
 * Parse JSON will allow you to parse the following fields from a JSON request  
*/
void parseJSON(char *request, struct creds *cred) {
    const nx_json* json = nx_json_parse_utf8(request);
    if (json){
        if (nx_json_get(json,"username") != NX_JSON_NULL){
            cred->username =  nx_json_get(json,"username")->text_value;
            //perror("parsed json username");
        }//end
        if (nx_json_get(json,"password") != NX_JSON_NULL){
            cred->password =  nx_json_get(json,"password")->text_value;
            //perror("parsed json password");
        }
        if (nx_json_get(json,"host") != NX_JSON_NULL){
            cred->host = nx_json_get(json,"host")->text_value;
            //perror("parsed json host");
        }
        if (nx_json_get(json,"port") != NX_JSON_NULL){
            cred->port = atoi(nx_json_get(json,"port")->text_value);
            //perror("parsed json port");
        }//end
        if (nx_json_get(json,"command") != NX_JSON_NULL){
            cred->command = nx_json_get(json,"command")->text_value;
            ////perror("parsed json command");
        }//end
        if (nx_json_get(json,"filename") != NX_JSON_NULL){
            cred->filename = nx_json_get(json,"filename")->text_value;
            ////perror("parsed json filename");
        }//end
        if (nx_json_get(json,"filebytes") != NX_JSON_NULL){
            cred->filebytes = nx_json_get(json,"filebytes")->text_value;
            ////perror("parsed json filebytes");
        }//end 
        if (nx_json_get(json,"vfs") != NX_JSON_NULL){
            cred->is_vfs = atoi(nx_json_get(json,"vfs")->text_value);
            //perror("parsed json vfs");
        }//end 
        if (nx_json_get(json,"dirpath") != NX_JSON_NULL){
            cred->dirpath = nx_json_get(json,"dirpath")->text_value;
            //perror("parsed json dirpath");
        }
        if (nx_json_get(json,"filepath") != NX_JSON_NULL){
            cred->filepath = nx_json_get(json,"filepath")->text_value;
            //perror("parsed json filepath");
        }
    }
    nx_json_free(json);
    //perror("done");
}