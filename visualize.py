import matplotlib.pyplot as plt
import networkx as nx


f = open("graph.txt",'r')


ini = []
non = []
edges = []
N = 0

for line in f:

  # print (line)
  N += 1
  node_info = line.split(':')
  neighbours = node_info[1]
  node = int(node_info[0][0])
  flag = node_info[0][2]
  neighbours = (neighbours.strip()).split()

  if (flag == 'i'):
    ini.append(node)
  else:
    non.append(node)

  for neighbour in neighbours:
    Neighbour = int (neighbour)
    if node < Neighbour:
      edges.append((node,Neighbour))

# print (ini)
# print (non)
# print (edges)
# print (N)

G=nx.empty_graph(N)
pos=nx.spring_layout(G)

nx.draw_networkx_nodes(G,pos,
                       nodelist=ini,
                       node_color='r',
                       node_size=500,
                       alpha=0.8)

nx.draw_networkx_nodes(G,pos,
                       nodelist=non,
                       node_color='#00b4d9',
                       node_size=500,
                       alpha=0.8)

nx.draw_networkx_edges(G,pos,edgelist=edges,width=2,alpha=0.5)

labels={}
for i in range(N):
  labels[i] = i

nx.draw_networkx_labels(G,pos,labels,font_size=16)

plt.axis('off')
plt.savefig("Graph.png")
plt.show()