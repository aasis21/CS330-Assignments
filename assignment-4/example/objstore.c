#include "lib.h"

#define MAX_OBJS 16
struct object{
     long id;
     long size;
     int cache_index;
     int dirty;
     char key[64];
};

struct object *objs;

#define malloc_4k(x) do{\
                         (x) = mmap(NULL, BLOCK_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);\
                         if((x) == MAP_FAILED)\
                              (x)=NULL;\
                     }while(0); 
#define free_4k(x) munmap((x), BLOCK_SIZE)

#ifdef CACHE         // CACHED implementation
static void init_object_cached(struct object *obj)
{
           obj->cache_index = -1;
           obj->dirty = 0;
           return;
}
static void remove_object_cached(struct object *obj)
{
           obj->cache_index = -1;
           obj->dirty = 0;
          return;
}
static int find_read_cached(struct objfs_state *objfs, struct object *obj, char *user_buf, int size)
{
         char *cache_ptr = objfs->cache + (obj->id << 12);
         if(obj->cache_index < 0){  /*Not in cache*/
              if(read_block(objfs, obj->id, cache_ptr) < 0)
                       return -1;
              obj->cache_index = obj->id;
             
         }
         memcpy(user_buf, cache_ptr, size);
         return 0;
}
/*Find the object in the cache and update it*/
static int find_write_cached(struct objfs_state *objfs, struct object *obj, const char *user_buf, int size)
{
         char *cache_ptr = objfs->cache + (obj->id << 12);
         if(obj->cache_index < 0){  /*Not in cache*/
              if(read_block(objfs, obj->id, cache_ptr) < 0)
                       return -1;
              obj->cache_index = obj->id;
             
         }
         memcpy(cache_ptr, user_buf, size);
         obj->dirty = 1;
         return 0;
}
/*Sync the cached block to the disk if it is dirty*/
static int obj_sync(struct objfs_state *objfs, struct object *obj)
{
   char *cache_ptr = objfs->cache + (obj->id << 12);
   if(!obj->dirty)
      return 0;
   if(write_block(objfs, obj->id, cache_ptr) < 0)
      return -1;
    obj->dirty = 0;
    return 0;
}
#else  //uncached implementation
static void init_object_cached(struct object *obj)
{
   return;
}
static void remove_object_cached(struct object *obj)
{
     return ;
}
static int find_read_cached(struct objfs_state *objfs, struct object *obj, char *user_buf, int size)
{
   void *ptr;
   malloc_4k(ptr);
   if(!ptr)
        return -1;
   if(read_block(objfs, obj->id, ptr) < 0)
       return -1;
   memcpy(user_buf, ptr, size);
   free_4k(ptr);
   return 0;
}
static int find_write_cached(struct objfs_state *objfs, struct object *obj, const char *user_buf, int size)
{
   void *ptr;
   malloc_4k(ptr);
   if(!ptr)
        return -1;
   memcpy(ptr, user_buf, size);
   if(write_block(objfs, obj->id, ptr) < 0)
       return -1;
   free_4k(ptr);
   return 0;
}
static int obj_sync(struct objfs_state *objfs, struct object *obj)
{
   return 0;
}
#endif

/*
Returns the object ID.  -1 (invalid), 0, 1 - reserved
*/
long find_object_id(const char *key, struct objfs_state *objfs)
{
    int ctr;
    struct object *obj = objs;
    for(ctr=0; ctr < MAX_OBJS; ++ctr){
          if(obj->id && !strcmp(obj->key, key))
              return obj->id;
          obj++;
    }      
    return -1;   
}

/*
  Creates a new object with obj.key=key. Object ID must be >=2.
  Must check for duplicates.

  Return value: Success --> object ID of the newly created object
                Failure --> -1
*/

long create_object(const char *key, struct objfs_state *objfs){
    
    int ctr;
    struct object *obj = objs;
    struct object *free = NULL; 
    for(ctr=0; ctr < MAX_OBJS; ++ctr){
          if(!obj->id && !free){
                free = obj;
                free->id = ctr+2;
          }
          else if(obj->id && !strcmp(obj->key, key)){
                return -1;
          }
          obj++;
    }      
    
    if(!free){
               dprintf("%s: objstore full\n", __func__);
               return -1;
    } 
    strcpy(free->key, key);
    init_object_cached(obj);
    return free->id;
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
    int ctr;
    struct object *obj = objs;
    for(ctr=0; ctr < MAX_OBJS; ++ctr){
          if(obj->id && !strcmp(obj->key, key)){
               remove_object_cached(obj);
               obj->id = 0;
               obj->size = 0;
               return 0;
          }
          obj++;
    }      
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
   struct object *obj = objs + objid - 2;
   dprintf("%d: Size of struct object\n", sizeof(*obj));
   if(obj->id != objid)
       return -1;
   if(size > BLOCK_SIZE)
        return -1;
   dprintf("Doing write size = %d\n", size);
   if(find_write_cached(objfs, obj, buf, size) < 0)
       return -1; 
   obj->size = size;
   return size;
}

/*
  Reads the content of the object onto the buffer with objid = objid.
  Return value: Success --> #of bytes written
                Failure --> -1
*/
long objstore_read(int objid, char *buf, int size, struct objfs_state *objfs)
{
   struct object *obj = objs + objid - 2;
   if(objid < 2)
       return -1;
   if(obj->id != objid)
       return -1;
   dprintf("Doing read size = %d\n", size);
   if(find_read_cached(objfs, obj, buf, size) < 0)
       return -1; 
   return size;
}

/*
  Reads the object metadata for obj->id = objid.
  Fillup buf->st_size and buf->st_blocks correctly
  See man 2 stat 
*/
int fillup_size_details(struct stat *buf)
{
   struct object *obj = objs + buf->st_ino - 2;
   if(buf->st_ino < 2 || obj->id != buf->st_ino)
       return -1;
   buf->st_size = obj->size;
   buf->st_blocks = obj->size >> 9;
   if(((obj->size >> 9) << 9) != obj->size)
       buf->st_blocks++;
   return 0;
}

/*
   Set your private pointeri, anyway you like.
*/
int objstore_init(struct objfs_state *objfs)
{
   int ctr;
   struct object *obj = NULL;
   malloc_4k(objs);
   if(!objs){
       dprintf("%s: malloc\n", __func__);
       return -1;
   }
   if(read_block(objfs, 0, (char *)objs) < 0)
       return -1;
   obj = objs;
   for(ctr=0; ctr < MAX_OBJS; ctr++, obj++){
      if(obj->id)
          init_object_cached(obj);
   }
   objfs->objstore_data = objs;
   dprintf("Done objstore init\n");
   return 0;
}

/*
   Cleanup private data. FS is being unmounted
*/
int objstore_destroy(struct objfs_state *objfs)
{
   int ctr;
   struct object *obj = objs;
   for(ctr=0; ctr < MAX_OBJS; ctr++, obj++){
            if(obj->id)
                obj_sync(objfs, obj);
   }
   if(write_block(objfs, 0, (char *)objs) < 0)
       return -1;
   free_4k(objs);
   objfs->objstore_data = NULL;
   dprintf("Done objstore destroy\n");
   return 0;
}
