Quine-Cluskey Algorithm
=========================

Version: 0.2 
Date: 2012-10-03

Licence
=======
Can be used freely even commercially (Public Domain)
The author assumes no liability! Use this at your own risk!

Contact
=======
mail@stefanmoebius.de

Legend
======

_
y	means (NOT y)

+	means OR conjunction

e.g.
 _
yz means the case when y = 1 and z = 0.

Sample
======

Here a simple sample for a NAND operator:


Enter number of variables: 2
Please enter desired results: ( 0 or 1)


yz = 0

_
yz = 1

 _
yz = 1

__
yz = 1


Result:

_   _
y + z

