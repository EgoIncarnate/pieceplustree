# PiecePlusTree

I was reading up on the "PieceTable" design recently and thought I'd have a go at implementing one.
After about 5 minutes of implementing it, I decided I wanted to try something different.
The O(n) nature of the simple implementation leaves some performance on the table.

So I quickly checked to see if anyone had written about merging a Red-Black tree and a PieceTable to avoid the O(n) scans.
Turns out yes, the trusty old (and might I add one of my favorite editors) AbiWord did this many years ago.
However, I'm not sure if it ever landed because their basic implementation had so many clever tricks in it.
But I digress.

Red-Black trees are fun, but they still involve a lot of memory allocations for nodes due to their simple binary branching.
I wanted something like an N-ary tree so I could fit more items with less memory allocations.
Besides, I prefer cacheline packed data-structures over pointer chasing trees.

However, one of the most common operations in a PieceTable is to get the whole range of the buffer.
You use this all the time when you save a document, or copy the whole buffer to send to a code diagnostic engine, etc.
Having to walk back up the tree in this case can be cost prohibitive.

The B+Tree (B plus tree) has a feature called "Linked Leaves" where each of the leaves of the tree have a pointer to the previous and next leaf.
This is very useful when you want a linear scan of the key space because you just look at each element in the leaf, then jump to the next leaf.
You can also use it to scan backwards, should that be necessary.

Instead of storing calculated lengths in the internal and leaf nodes we store them with the parent.
Note that we do not store offsets within the nodes (with the exception of the PieceTableEntry, but that is external data as far as we are concerned).
We must walk the tree to calculate the offset by summing the length information of each element.
This is not expensive because we are already walking the tree as we search to find the target node.
Calculating the offset prevents us from using `bsearch()` within the node, but given the cacheline friendliness, it's not cumbersome.

The "stupid" implementation I started with just kept things as an array and would `memmove()` to keep items sorted.
When doing lots of insertions we would spend a great deal of time just shuffling data.
Then I remembered a trick I'd seen in some B-Tree indexes years ago.
You can grow data from one edge of the leaf and sort the elements using the other end of the leaf.
That way you aren't `memmove()`ing, but instead just minimalling shuffling a queue where you already have O(1) access to the effected queue link.

Each queue element is in the matching bucket as the real data.
It has a `previous` and `next` bucket identifier and generally works like a linked-list-based queue but without pointers.

We do have one invariant that must be kept across all nodes.
The data elements must not have gaps in them (so we know how to quickly place new elements without finding a free slot).
Since we have a queue for sorting, we can use fast-removal in the array by taking the tail element and moving it into the removed element position.
Then we update the queue which is `O(1)` as we already know our raw bucket position.

## TODO

 * While we have delete support, we don't yet have tree node delete phases implemented.
   I expect this to be tricky, so I'm still thinking about it.
 * To make this useful, we'll need some useful operation tracking added (to aid in undo/redo).

