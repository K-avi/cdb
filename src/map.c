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
       
        if(!head->head){
            failure = bucket_insert(head, key, value);
            error_handler(failure, "map_insert bucket_insert", failure,  pthread_mutex_unlock(&(head->lock)););
            pthread_mutex_unlock(&(head->lock));
            return ERR_OK;
        }

        if(is_empty(head)){
           
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

errflag_t map_remove(s_map *map, s_key *key){
    
    return ERR_OK;
}//not done

errflag_t map_lookup(s_map *map, s_key *key, s_value **value_ret){
    return ERR_OK;
}//not done

errflag_t map_free(s_map *map){
    return ERR_OK;
}//not done

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
        bucket_print(&(map->bucket_heads[i]));
        printf("----------------\n");
    }
}
#endif
