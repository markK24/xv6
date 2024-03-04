// Buffer cache.
//
// The buffer cache is a linked list of buf structures holding
// cached copies of disk block contents.  Caching disk blocks
// in memory reduces the number of disk reads and also provides
// a synchronization point for disk blocks used by multiple processes.
//
// Interface:
// * To get a buffer for a particular disk block, call bread.
// * After changing buffer data, call bwrite to write it to disk.
// * When done with the buffer, call brelse.
// * Do not use the buffer after calling brelse.
// * Only one process at a time can use a buffer,
//     so do not keep them longer than necessary.


#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "riscv.h"
#include "defs.h"
#include "fs.h"
#include "buf.h"

#define BUCKETS 7

struct {
  struct spinlock bucket_lock[BUCKETS];
  struct buf buf[NBUF];
} bcache;

void
binit(void)
{
  for (int i = 0; i < BUCKETS; i++) {
    initlock(&bcache.bucket_lock[i], "bcache-bucket");
  }
  for (int i = 0; i < NBUF; i++) {
    initsleeplock(&bcache.buf[i].lock, "buffer");
  }
}

// Look through buffer cache for block on device dev.
// If not found, allocate a buffer.
// In either case, return locked buffer.
static struct buf*
bget(uint dev, uint blockno)
{
  int bucket = blockno % BUCKETS;

  acquire(&bcache.bucket_lock[bucket]);

  // Is the block already cached?
  for (int i = bucket; i < NBUF; i += BUCKETS) {
    struct buf *b = &bcache.buf[i];
    if (b->dev == dev && b->blockno == blockno) {
      b->refcnt++;
      release(&bcache.bucket_lock[bucket]);
      acquiresleep(&b->lock);
      return b;
    }
  }

  // Not cached.
  for(int i = bucket; i < NBUF; i += BUCKETS){
    struct buf *b = &bcache.buf[i];
    if(b->refcnt == 0) {
      b->dev = dev;
      b->blockno = blockno;
      b->valid = 0;
      b->refcnt = 1;
      release(&bcache.bucket_lock[bucket]);
      acquiresleep(&b->lock);
      return b;
    }
  }
  panic("bget: no buffers in bucket");
}

// Return a locked buf with the contents of the indicated block.
struct buf*
bread(uint dev, uint blockno)
{
  struct buf *b;

  b = bget(dev, blockno);
  if(!b->valid) {
    virtio_disk_rw(b, 0);
    b->valid = 1;
  }
  return b;
}

// Write b's contents to disk.  Must be locked.
void
bwrite(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("bwrite");
  virtio_disk_rw(b, 1);
}

// Release a locked buffer.
// Move to the head of the most-recently-used list.
void
brelse(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("brelse");

  releasesleep(&b->lock);

  int bucket = b->blockno % BUCKETS;

  acquire(&bcache.bucket_lock[bucket]);
  b->refcnt--;
  release(&bcache.bucket_lock[bucket]);
}

void
bpin(struct buf *b) {
  int bucket = b->blockno % BUCKETS;
  acquire(&bcache.bucket_lock[bucket]);
  b->refcnt++;
  release(&bcache.bucket_lock[bucket]);
}

void
bunpin(struct buf *b) {
  int bucket = b->blockno % BUCKETS;
  acquire(&bcache.bucket_lock[bucket]);
  b->refcnt--;
  release(&bcache.bucket_lock[bucket]);
}


