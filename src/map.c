#include "map.h"
#include "err_handler.h"
#include "key.h"
#include "value.h"

#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <threads.h>
#include <stdbool.h>

#define TOMB_FLAG 0x2
#define EMPTY_FLAG 0x1
#define is_tombstone(buckethead) (buckethead->flags & TOMB_FLAG)
#define is_empty(buckethead) (buckethead->flags & EMPTY_FLAG)

errflag_t map_init(uint32_t nb_buckets, s_map *map){
    def_err_handler(!map, "map_init map", ERR_NULL);
    def_err_handler(!nb_buckets, "map_init nb_buckets", ERR_INVALID);

    map->nb_buckets = nb_buckets;

    map->bucket_heads = calloc(nb_buckets, sizeof(s_buckethead));
    def_err_handler(!map->bucket_heads, "map_init map->buckets", ERR_ALLOC);

    for(uint32_t i = 0; i < nb_buckets; i++){
        pthread_mutex_init(&(map->bucket_heads[i].lock), NULL);
        map->bucket_heads[i].flags = EMPTY_FLAG;
    }

    return ERR_OK;
}//tested; should be ok; initializes the map with nb_buckets bucket_heads


static errflag_t bucket_create(s_mapbucket *bucket, s_key *key, s_value *value){
    def_err_handler(!bucket, "bucket_create bucket", ERR_NULL);
    def_err_handler(!key, "bucket_create key", ERR_NULL);
    def_err_handler(!value, "bucket_create value", ERR_NULL);

    bucket->key = calloc(1, sizeof(s_key));
    def_err_handler(!bucket->key, "bucket_create bucket->key", ERR_ALLOC);

    errflag_t failure = key_duplicate(key, bucket->key);
    def_err_handler(failure, "bucket_create key_duplicate", failure);

    bucket->value = calloc(1, sizeof(s_value));
    def_err_handler(!bucket->value, "bucket_create bucket->value", ERR_ALLOC);

    failure = value_dup(value, bucket->value);
    def_err_handler(failure, "bucket_create value_dup", failure);

    pthread_mutex_init(&(bucket->lock), NULL);

    return ERR_OK;
}//tested; should be ok; creates a bucket with a key and a value


static void free_bucket(s_mapbucket *bucket){
    if(!bucket) return ; 
    
    key_free(bucket->key); 
    value_free(bucket->value);

    free(bucket->key);
    free(bucket->value);
    pthread_mutex_destroy(&(bucket->lock));

    free(bucket);
}//doesnt free the next bucket; only the key and the value of the bucket


static errflag_t bucket_insert(s_buckethead *head, s_key *key, s_value *value){
    def_err_handler(!head, "bucket_insert head", ERR_NULL);
    def_err_handler(!key, "bucket_insert key", ERR_NULL);
    def_err_handler(!value, "bucket_insert value", ERR_NULL);

    s_mapbucket *bucket = head->head;
    s_mapbucket *new_head = calloc(1, sizeof(s_mapbucket));

    errflag_t failure = bucket_create(new_head, key, value);
    error_handler(failure, "bucket_insert bucket_create", failure, free_bucket(new_head););

    new_head->next = bucket;
    head->head = new_head;

    return ERR_OK;
}//tested; should be ok; inserts a key value pair in a bucket

errflag_t map_insert(s_map *map, s_key *key, s_value *value){
    def_err_handler(!map, "map_insert map", ERR_NULL);
    def_err_handler(!key, "map_insert key", ERR_NULL);
    def_err_handler(!value, "map_insert value", ERR_NULL);

    uint32_t hashed_key = 0;
    errflag_t failure = key_hash(key, &hashed_key);
    def_err_handler(failure, "map_insert key_hash", failure);

    uint32_t bucket_index = hashed_key % map->nb_buckets;
    uint32_t start_index = bucket_index;

    s_buckethead *head = &(map->bucket_heads[bucket_index]);

    do{
        pthread_mutex_lock(&(head->lock));

        if(is_empty(head)){ 
            head->flags &= ~EMPTY_FLAG;
            head->flags &= ~TOMB_FLAG;

            failure = bucket_insert(head, key, value);
            error_handler(failure, "map_insert bucket_insert", failure, pthread_mutex_unlock(&(head->lock)););
            pthread_mutex_unlock(&(head->lock));
            return ERR_OK;
        }

        if(!strncmp(head->head->key->key, key->key, key->key_size)){
            failure = bucket_insert(head, key, value);
            error_handler(failure, "map_insert bucket_insert", failure, pthread_mutex_unlock(&(head->lock)););
            pthread_mutex_unlock(&(head->lock));
            return ERR_OK;
        }

        pthread_mutex_unlock(&(head->lock));

        start_index = (start_index + 1) % map->nb_buckets;
        head = &(map->bucket_heads[start_index]);

    }while(bucket_index != start_index );

    do{//insert into tombstone if no empty bucket

        pthread_mutex_lock(&(head->lock));
        if(is_tombstone(head)){
            head->flags &= ~EMPTY_FLAG;
            head->flags &= ~TOMB_FLAG;

            failure = bucket_insert(head, key, value);
            error_handler(failure, "map_insert bucket_insert", failure,pthread_mutex_unlock(&(head->lock)););
            pthread_mutex_unlock(&(head->lock));
            return ERR_OK;
        }

        pthread_mutex_unlock(&(head->lock));
       
        start_index = (start_index + 1) % map->nb_buckets;
        head = &(map->bucket_heads[start_index]);
    }while(bucket_index != start_index );

    return ERR_MAPFULL;
}//tested; seems ok 
//insert is kinda tricky since I want every version of a key to be stored on the same list 
//very bad performance if the map is saturated / has a lot of tombstones
// -> do smtg where u turn tombstones into empty buckets when a certain threshold is reached or smth
//might have concurrency issues 

errflag_t map_lookup(s_map *map, s_key *key, s_value *value_ret){
    def_err_handler(!map, "map_lookup map", ERR_NULL);
    def_err_handler(!key, "map_lookup key", ERR_NULL);
    def_err_handler(!value_ret, "map_lookup value_ret", ERR_NULL);

    uint32_t hashed_key = 0;
    errflag_t failure = key_hash(key, &hashed_key);
    def_err_handler(failure, "map_lookup key_hash", failure);

    uint32_t bucket_index = hashed_key % map->nb_buckets;
    uint32_t start_index = bucket_index;

    s_buckethead *head = &(map->bucket_heads[bucket_index]);

    do{
        pthread_mutex_lock(&(head->lock));
        if (! (head->flags & (TOMB_FLAG | EMPTY_FLAG) ) ){ 
            if(!strncmp(head->head->key->key, key->key, key->key_size)){
                //search the list for the closest timestamp to the one in the key

                s_mapbucket *bucket = head->head;
                //iterate to fetch the closest version to the one asked j'
                while(bucket->key->ts > key->ts && bucket->next){
                    //keys are sorted from highest to lowest timestamp bc of head insert
                    //if this property is not respected, the function will not work 
                    //however sorted insert in linked list is very easy so it should be a quick 
                    //fix if need be
                    if(bucket->next->key->ts <= key->ts){
                        bucket = bucket->next;
                        
                        break;
                    }
                    bucket = bucket->next;
                }

                if(bucket->key->ts <= key->ts){
                    failure = value_dup(bucket->value, value_ret);
                    error_handler(failure, "map_lookup value_dup", failure, pthread_mutex_unlock(&(head->lock)););
                    pthread_mutex_unlock(&(head->lock));
                    return ERR_OK;
                }

                pthread_mutex_unlock(&(head->lock));
                value_ret->as = UNKNOWKN;
                value_ret->val.u64 =  0 ;

                return ERR_OK;
            }
        }
        pthread_mutex_unlock(&(head->lock));

        start_index = (start_index + 1) % map->nb_buckets;
        head = &(map->bucket_heads[start_index]);
    }while(bucket_index != start_index || !is_empty(head) );

    value_ret->as = UNKNOWKN;
    value_ret->val.u64 =  0 ;

    return ERR_OK;
}//tested ; seems ok 
//I don't like the fact that I lock the whole bucket list when I only need to lock the bucket I'm looking at

errflag_t map_remove(s_map *map, s_key *key){
    def_err_handler(!map, "map_remove map", ERR_NULL);
    def_err_handler(!key, "map_remove key", ERR_NULL);

    uint32_t hashed_key = 0;
    errflag_t failure = key_hash(key, &hashed_key);
    def_err_handler(failure, "map_remove key_hash", failure);

    uint32_t bucket_index = hashed_key % map->nb_buckets;
    uint32_t start_index = bucket_index;

    s_buckethead *head = &(map->bucket_heads[bucket_index]);

    do{
        pthread_mutex_lock(&(head->lock));
        if (!(head->flags & (TOMB_FLAG | EMPTY_FLAG) ) ){ 
            if(!strncmp(head->head->key->key, key->key, key->key_size)){
                s_mapbucket *bucket = head->head;
                s_mapbucket *prev = NULL;

                while(bucket){
                    
                    if(bucket->key->ts == key->ts){
                        if(prev){
                            prev->next = bucket->next;
                        }else{
                            head->head = bucket->next;

                            if(!head->head){//set to tombstoen if the bucket list is empty after removal
                                head->flags |= TOMB_FLAG;
                            }
                        }
                        free_bucket(bucket);
                        pthread_mutex_unlock(&(head->lock));
                        return ERR_OK;
                    }

                    prev = bucket;
                    bucket = bucket->next;
                }
            }
        }
        pthread_mutex_unlock(&(head->lock));

        start_index = (start_index + 1) % map->nb_buckets;
        head = &(map->bucket_heads[start_index]);

    }while(bucket_index != start_index || !is_empty(head) );

    return ERR_OK;
}//not tested; should be ok; removes a key from the map if the key,timestamp pair exists

errflag_t map_delete_key(s_map *map, s_key *key){
    def_err_handler(!map, "map_delete_key map", ERR_NULL);
    def_err_handler(!key, "map_delete_key", ERR_NULL);

    uint32_t hashed_key = 0;
    errflag_t failure = key_hash(key, &hashed_key);
    def_err_handler(failure, "map_remove key_hash", failure);

    uint32_t bucket_index = hashed_key % map->nb_buckets;
    uint32_t start_index = bucket_index;

    s_buckethead *head = &(map->bucket_heads[bucket_index]);

    do{
        pthread_mutex_lock(&(head->lock));

        if (! (head->flags & (TOMB_FLAG | EMPTY_FLAG) ) ){ 
            if(!strncmp(head->head->key->key, key->key, key->key_size)){
                s_mapbucket *bucket = head->head;
                s_mapbucket *next = NULL;

                while(bucket){
                    next = bucket->next;    
                    free_bucket(bucket);        
                    bucket = next;
                }
                head->flags |= TOMB_FLAG;

                pthread_mutex_unlock(&(head->lock));
                return ERR_OK;
            }       
        }
        pthread_mutex_unlock(&(head->lock));

        bucket_index = (bucket_index + 1) % map->nb_buckets;

        if(bucket_index == start_index) break;

        head = &(map->bucket_heads[bucket_index]);

    }while( (bucket_index != start_index) || !is_empty(head) );

    return ERR_OK;
}//tested; seems ok 
//deletes every key/value for the given key, whatever the timestamp

errflag_t map_free(s_map *map){

    if(!map) return ERR_OK;

    if(map->bucket_heads){
        for(uint32_t i = 0; i < map->nb_buckets; i++){

            if( ! (map->bucket_heads[i].flags & (TOMB_FLAG | EMPTY_FLAG))){
                s_mapbucket *bucket = map->bucket_heads[i].head;
                while(bucket){
                    s_mapbucket *next = bucket->next;
                    free_bucket(bucket);
                    bucket = next;              
                }
            }
            pthread_mutex_destroy(&(map->bucket_heads[i].lock));
        }
    }
    free(map->bucket_heads);
    map->nb_buckets = 0 ; 
    map->bucket_heads = NULL;
    return ERR_OK;
}//tested; ok ; not thread safe; will break stuff if used while map is still being accessed by other threads

#ifdef debug 
static void bucket_print(s_buckethead *head){
    if(!head) return ; 

    s_mapbucket *bucket = head->head;

    while(bucket){
        key_print(bucket->key);
        value_print(bucket->value);
        bucket = bucket->next;
    }
}

void map_print(s_map *map){

    for (uint32_t i = 0 ; i < map->nb_buckets; i++) {
        printf("bucket %u\n", i);
        if(map->bucket_heads[i].flags & (EMPTY_FLAG | TOMB_FLAG)){
            printf("empty\n");
          
        }else{
            bucket_print(&(map->bucket_heads[i]));
        }
        printf("----------------\n");
    }
}
#endif
