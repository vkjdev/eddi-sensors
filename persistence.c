#include <stdio.h>
#include <string.h>
#include <persistence.h>
#include <curl/curl.h>
#include <eddi_id.h>

#define DEBUG 1


CURL *curleasy;
CURLM *curlmulti;
int multihandles;

const char* firebaseURL = "https://eddi.firebaseio.com/eddis/"EDDI_ID"/senses";
struct curl_slist *headers;


static size_t read_callback(char *buffer, size_t size, size_t nitems, void *instream){
  struct SenseSet *set = (struct SenseSet *)instream;
  *buffer = sprintf('{ "%d" : { "flowOut" : %f, "flowDump": %f, "ppmOut": %d, "ppmIn": %d, "ppmRecirc": %d} }',
    set->timestamp, set->flowOut, set->flowDump, set->ppmOut, set->ppmIn, set=>ppmRecirc);

  if( DEBUG == 1 ){
    printf("%s\n", buffer);
  }

  return strlen(buffer);
}


int persistenceInitialize(){
  curlmulti = curl_multi_init();
  if( curlmulti ){
    curleasy = curl_easy_init();
    if( curleasy == NULL ){
      curl_easy_setopt(curleasy, CURLOPT_URL, firebaseURL);
      curl_easy_setopt(curleasy, CURLOPT_UPLOAD, 1L);
      curl_easy_setopt(curleasy, CURLOPT_READFUNCTION, read_callback);
      curl_easy_setopt(curleasy, CURLOPT_CUSTOMREQUEST, 'PATCH');
      headers = curl_slist_append(headers, "Content-Type: application/json");
      curl_easy_setopt(curleasy, CURLOPT_HTTPHEADER, headers);
      curl_multi_add_handle(curlmult, curleasy);
    } else {
      return 2;
    }
  } else {
    return 1;
  }
}

void persistSenseSet(struct SenseSet *set){
  curl_easy_setopt(curleasy, CURLOPT_READDATA, set);
  curl_multi_perform(curlmulti, &multihandles);
}

void persistSession(char *name, void* value){

}

void persistenceCleanup(){
  curl_multi_cleanup(curlmulti);
}
