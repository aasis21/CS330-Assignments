#include "lib.h"

#define BIT_ARRAY_SIZE(x) (x/8+(!!(x%8)))

#define HASHTABLE_SIZE  1100000 // 1.1 times
#define HNODE_SIZE 40

#define NUM_IBIT_BLK 32
#define NUM_DBIT_BLK 256
#define NUM_HT_BLK 




#define malloc_4k(x) do{\
                         (x) = mmap(NULL, BLOCK_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);\
                         if((x) == MAP_FAILED)\
                              (x)=NULL;\
                     }while(0); 

#define malloc_g(x,size) do{\
                         (x) = mmap(NULL, (size) , PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);\
                         if((x) == MAP_FAILED)\
                              (x)=NULL;\
                     }while(0); 

#define free_4k(x) munmap((x), BLOCK_SIZE)



typedef unsigned long u64;

struct inode{
     long inumber;
     char key[32];
     long size;
     int cache_index;
     int dirty;
     long direct_block[12];
     long indirect_block[8];
     
};

typedef struct hash_node{
    char key[32];
    char status;
    int inumber;
    // Size Of the Structure is 40 B;
} hnode ;

struct objfs_data{
    struct hash_node** hash_table ;
    u64** inode_bit_map;
    char** dnode_bit_map;
};


unsigned long hash(unsigned char *str){
    /* djb2  Hash function, Taken from http://www.cse.yorku.ca/~oz/hash.html */

    unsigned long hash = 5381;
    int c;

    while (c = *str++)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}

/*
Returns the object ID.  -1 (invalid), 0, 1 - reserved
*/
long find_object_id(const char *key, struct objfs_state *objfs)
{
    int key_index = hash(key) % HASHTABLE_SIZE ;

    long hnode_in_4k = BLOCK_SIZE / HNODE_SIZE; 
    long num_block_hash = (HASHTABLE_SIZE + hnode_in_4k - 1)/ hnode_in_4k;

    long block_num = key_index / hnode_in_4k;

    long block_index = key_index % hnode_in_4k;

    struct objfs_data *data = (struct objfs_data *)objfs->objstore_data;

    hnode ** hash_table = data->hash_table;

    int block_i = block_index;
    for(int b = block_num ; b < num_block_hash; b++){
        for(int i = block_i; i< hnode_in_4k; i++){
            if(hash_table[b][i].status = 0 ){
                // null -> not found
                return -1;
            }
            if(hash_table[b][i].status = 2 ){
                // data present
                if(hash_table[b][i].key && !strcmp(hash_table[b][i].key, key))
                    return hash_table[b][i].inumber;
            }   
        } 
        block_i = 0;
    }

    for(int b = 0; b < block_num; b++){
        for(int i = 0; i< hnode_in_4k; i++){
            if(hash_table[b][i].status = 0 ){
                // null -> not found
                return -1;
            }
            if(hash_table[b][i].status = 2 ){
                // data present
                if(hash_table[b][i].key && !strcmp(hash_table[b][i].key, key))
                    return hash_table[b][i].inumber;
            }   
        } 
    }

    for(int i = 0 ; i< block_index; i++){
        int b = block_num;
        if(hash_table[b][i].status = 0 ){
            // null -> not found
            return -1;
        }
        if(hash_table[b][i].status = 2 ){
            // data present
            if(hash_table[b][i].key && !strcmp(hash_table[b][i].key, key))
                return hash_table[b][i].inumber;
        }   
    }    
    
    return -1;   
}

/*
  Creates a new object with obj.key=key. Object ID must be >=2.
  Must check for duplicates.

  Return value: Success --> object ID of the newly created object
                Failure --> -1
*/




long create_object(const char *key, struct objfs_state *objfs)
{   
    long hnode_in_4k = BLOCK_SIZE / HNODE_SIZE; 
    long NUM_HT_BLK = (HASHTABLE_SIZE + hnode_in_4k - 1)/ hnode_in_4k;

    long OFFSET_IBIT = 2;
    long OFFSET_DBIT = OFFSET_IBIT +  NUM_IBIT_BLK;
    long OFFSET_HT = OFFSET_DBIT +  NUM_DBIT_BLK;
    long OFFSET_INODE = OFFSET_HT +  NUM_HT_BLK;

    long inumber = find_object_id(key, objfs);

    if(inumber != -1){
        return -1;
    }

    struct objfs_data *data = (struct objfs_data *)objfs->objstore_data;

    u64 **inode_bit_map = data->inode_bit_map;

    int b_e, i_e, j_e;

    int bit_full = 1;
    for(int b = 0 ; b < 8 ; b++ ){
        int break_b = 0;
        for(int i=0; i < 512; i++){
            int break_i = 0;
            u64 bits = inode_bit_map[b][i];
            if( bits < ((1<<64) -1)){
                for(int j = 0; j< 64; j++){
                   if( ( bits & (1 << j) ) == 0 ){
                       j_e = j;
                       break_i = 1;
                       break;
                   }
                }  
            }
            if(break_i == 1){
                i_e = i;
                break_b = 1;
                break;
            }
        }
        if(break_b == 1){
            b_e = b;
            bit_full = 0;
            break;
        }
    }

    if(bit_full == 1){
        return -1;
    }

    inumber = (b_e * (1 << 15)) + (i_e * 64) + j_e;


    // find a empty inode block in inode bitmap




    return -1;
}
/*
  One of the users of the object has dropped a reference
  Can be useful to implement caching.
  Return value: Success --> 0
                Failure --> -1
*/
long release_object(int objid, struct objfs_state *objfs)
{
    return 0;
}

/*
  Destroys an object with obj.key=key. Object ID is ensured to be >=2.

  Return value: Success --> 0
                Failure --> -1
*/
long destroy_object(const char *key, struct objfs_state *objfs)
{
    return -1;
}

/*
  Renames a new object with obj.key=key. Object ID must be >=2.
  Must check for duplicates.  
  Return value: Success --> object ID of the newly created object
                Failure --> -1
*/

long rename_object(const char *key, const char *newname, struct objfs_state *objfs)
{
   
   return -1;
}

/*
  Writes the content of the buffer into the object with objid = objid.
  Return value: Success --> #of bytes written
                Failure --> -1
*/

long objstore_write(int objid, const char *buf, int size, struct objfs_state *objfs)
{
   return -1;
}

/*
  Reads the content of the object onto the buffer with objid = objid.
  Return value: Success --> #of bytes written
                Failure --> -1
*/
long objstore_read(int objid, char *buf, int size, struct objfs_state *objfs)
{
   return -1;
}

/*
  Reads the object metadata for obj->id = buf->st_ino
  Fillup buf->st_size and buf->st_blocks correctly
  See man 2 stat 
*/
int fillup_size_details(struct stat *buf)
{
   return -1;
}

/*
   Set your private pointeri, anyway you like.
*/
int objstore_init(struct objfs_state *objfs)
{   

    long hnode_in_4k = BLOCK_SIZE / HNODE_SIZE; 
    long NUM_HT_BLK = (HASHTABLE_SIZE + hnode_in_4k - 1)/ hnode_in_4k;

    long OFFSET_IBIT = 2;
    long OFFSET_DBIT = OFFSET_IBIT +  NUM_IBIT_BLK;
    long OFFSET_HT = OFFSET_DBIT +  NUM_DBIT_BLK;
    long OFFSET_INODE = OFFSET_HT +  NUM_HT_BLK;
    

    
    // READING INODE BITMAP FORM DISK TO MEMORY
    // 8 BLOCKS RESERVERVED IN START
    u64 **inode_bitmap = (u64**)malloc(NUM_IBIT_BLK *sizeof(u64 *));
    for( int i=0; i < NUM_IBIT_BLK ; i++ ){
        char * ibit_block = NULL;
        malloc_4k(ibit_block);
        if(!ibit_block){
            dprintf("%s: malloc error\n", __func__);
            return -1;
        }
        inode_bitmap[i] = ibit_block;
        long offset = OFFSET_IBIT + i;
        char *buf = (char *)ibit_block;
        if( read_block(objfs, offset , buf ) < 0){
            return -1;
        }
    }

    // READING DATA BITMAP FORM DISK TO MEMORY
    // 2 << 8 BLOCKS RESERVERVED IN START
    char **dnode_bitmap = (char**)malloc(NUM_DBIT_BLK * sizeof(char *));
    for( int i=0; i < NUM_DBIT_BLK ; i++ ){
        char * dbit_block = NULL;
        malloc_4k(dbit_block);
        if(!dbit_block){
            dprintf("%s: malloc error\n", __func__);
            return -1;
        }
        dnode_bitmap[i] = dbit_block;
        long offset = OFFSET_DBIT + i;
        char *buf = (char *)dbit_block;
        if( read_block(objfs, offset , buf ) < 0){
            return -1;
        }
    }

    // READING HASTABLE FORM DISK TO MEMORY
    

    hnode **hash_table = NULL;
    malloc_g(hash_table, NUM_HT_BLK * sizeof(hnode *));

    for( int i=0; i < NUM_HT_BLK ; i++ ){
        struct hash_node * hashb = NULL;
        malloc_4k(hashb);
        if(!hashb){
            dprintf("%s: malloc error\n", __func__);
            return -1;
        }
        hash_table[i] = hashb;
        long offset = OFFSET_HT + i;
        char *buf = (char *)hashb ;
        if( read_block(objfs, offset , buf ) < 0){
            return -1;
        }
    }

    dprintf("SIZE: %d", sizeof(hnode));
    struct objfs_data *data = (struct objfs_data *)malloc(sizeof(struct objfs_data ));
    data->hash_table = hash_table;
    data->inode_bit_map = inode_bitmap;
    data->dnode_bit_map = dnode_bitmap;

    objfs->objstore_data = data;
    
    
    dprintf("Done objstore init\n");
    return 0;
}

/*
   Cleanup private data. FS is being unmounted
*/
int objstore_destroy(struct objfs_state *objfs)
{
   dprintf("Done objstore destroy\n");
   return 0;
}
