This is a modified version of v2.
This version also handles multi-connection issue. The previous machineb only expects to connects to machinea first, 
but will terminate if some other connection joins. But because this is a open network, there could be other connections. 