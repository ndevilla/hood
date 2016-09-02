# -*- coding: utf-8 -*-
import os
import sys
import hashlib

def keyhash(key):
    return int(hashlib.md5(key).hexdigest(), 16)

class Keypair:
    def __init__(self, key, val, dib=0):
        self.key  = key
        self.val  = val
        self.hsh  = keyhash(key)
        self.dib  = dib

class Hood:
    def __init__(self, cap=2):
        self.cap     = cap
        self.fill    = 0
        self.buckets = [None for x in range(self.cap)]

    def insert(self, kp):
        kp.dib=0
        while 1:
            #print 'inserting: key['+kp.key+'] val['+kp.val+']'
            # Look up slot
            pos = kp.hsh % self.cap
            kp.dib=0
            while 1:
                pos = pos % self.cap
                #print '   pos:', pos
                kloc = self.buckets[pos]
                if kloc == None:
                    #print '      free slot at dib=', kp.dib
                    # Free slot: insert
                    self.buckets[pos] = kp
                    self.fill+=1
                    break
                # Compare local and candidate dib
                if kloc.dib < kp.dib:
                    #print '      hood at', pos
                    # Insert in place
                    self.buckets[pos] = kp
                    # Re-insert kloc
                    #print ' displacing', kloc.key
                    self.insert(kloc)
                    break
                else:
                    #print '      not hooding', kloc.dib, kp.dib
                    pass
                # Continue searching
                kp.dib+=1
                pos+=1
            break 
        nh = resize(self)
        if nh:
            self.cap = nh.cap
            self.fill = nh.fill
            self.buckets = nh.buckets

    def lookup(self, key):
        kp = Keypair(key, None) 
        pos = kp.hsh % self.cap
        while 1:
            if self.buckets[pos] == None:
                return -1
            if self.buckets[pos].hsh == kp.hsh:
                return pos
            pos=(pos+1) % self.cap

    def find(self, key):
        pos = self.lookup(key)
        if pos>=0:
            return self.buckets[pos].val
        return None

    def delete(self, key):
        #print 'deleting:', key
        pos = self.lookup(key)
        #print 'lookup at', pos
        if pos<0:
            return
        while 1:
            # Delete keypair at pos
            self.buckets[pos]=None
            # Backtrack on later keypairs
            npos = (pos+1)%self.cap
            if self.buckets[npos] and self.buckets[npos].dib==0:
                break
            self.buckets[pos] = self.buckets[npos]
            if self.buckets[npos]==None:
                break
            self.buckets[pos].dib-=1
            pos = npos
        self.fill-=1

    def show(self):
        print '----- hood', self.fill, self.cap
        for kp in self.buckets:
            if kp:
                print 'key['+kp.key+'] val['+kp.val+']'
            #else:
                #print '--empty'

def resize(hood):
    if 3*hood.fill < 2*hood.cap:
        return None

    print 'resizing to', hood.cap*2
    newHood = Hood(hood.cap *2)
    for kp in hood.buckets:
        if kp:
            kp.dib=0
            newHood.insert(kp)
            #newHood.show()
    return newHood


def heavy():
    size=10240
    rh = Hood()
    print 'start insertion'
    for i in range(size):
        rh.insert(Keypair(str(i), str(i)))
    print 'stop insertion'

    print 'start lookup'
    for i in range(size):
        val = rh.find(str(i))
        if val != str(i):
            print 'lookup failed [%d][%s]' % (i, val)
    print 'stop lookup'
    #rh.show()

    print 'start delete'
    for i in range(size):
        rh.delete(str(i))
    print 'stop delete'

    print 'start lookup'
    for i in range(size):
        val = rh.find(str(i))
        if val != None:
            print 'lookup failed [%d][%s]' % (i, val)
    print 'stop lookup'
    rh.show()

def unit_test():
    rh = Hood()
    print '----- insertions'
    rh.insert(Keypair('1', 'one'))
    rh.insert(Keypair('2', 'two'))
    rh.insert(Keypair('3', 'three'))
    rh.insert(Keypair('4', 'four'))
    rh.insert(Keypair('5', 'five'))
    rh.insert(Keypair('6', 'six'))
    rh.insert(Keypair('7', 'seven'))
    rh.insert(Keypair('8', 'eight'))
    rh.insert(Keypair('9', 'nine'))
    rh.insert(Keypair('10', 'ten'))
    rh.show()

    print '----- lookups'
    print rh.find('1')
    print rh.find('2')
    print rh.find('3')
    print rh.find('4')
    print rh.find('5')
    print rh.find('6')
    print rh.find('7')
    print rh.find('8')
    print rh.find('9')
    print rh.find('10')
    print rh.find('11')

    print '----- delete'
    for i in ['1','2','3','4','5','6','7','8','9','10']:
        rh.delete(i)
        rh.show()
    rh.show()

if __name__=="__main__":
    heavy()

