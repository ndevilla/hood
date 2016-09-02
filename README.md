# hood
Hashmap based on Robin Hood hashing

Found this blog post recently on YNews:
http://www.sebastiansylvan.com/post/robin-hood-hashing-should-be-your-default-hash-table-implementation/

Sounds like fun and I had not heard of that algorithm before, so I had to
try it. The implementations in this repository are the result of my
attempts.

The Python version was only written as scaffolding to make sure I
understood the algorithm. The C version is based on what I previously wrote
in github.com/ndevilla/dict

Results are not brilliant: my Robin Hood implementation is a factor 2
slower than dict (based on Python dictionaries). Interested readers are
invited to check out:

- The YNews discussion https://news.ycombinator.com/item?id=12399989
- http://codecapsule.com/2013/11/11/robin-hood-hashing/
- http://codecapsule.com/2013/11/17/robin-hood-hashing-backward-shift-deletion/

Compile with 'make', run the executable to benchmark it against 10 million
elements.

Nicolas
