import matplotlib.pyplot as plt
import seaborn as sns
import numpy as np
sns.set()
import os

def process(logs_dir, save_dir):
    data = []
    types = []
    title = logs_dir.split('/')[-1]

    for file in os.listdir(logs_dir):
        filename = os.path.join(logs_dir, file)
        node_name = int(file.split('_')[0])
        
        data_dict = {
            "node_name": node_name,
            "sent": [],
            "recieved": []
        }

        with open(filename, "r") as f:
            for line in f.readlines():
                sl = line.split(' ')
                if 'sent' == sl[1]:
                    msg = {
                        "type": sl[4].strip(":)("),
                        "sender": int(sl[0]),
                        "receiver": int(sl[3])
                    }

                    data_dict["sent"].append(msg)
                    types.append(msg["type"])
                
                if 'recieved' == sl[1]:
                    msg = {
                        "type": sl[4].strip(":)("),
                        "sender": int(sl[3]),
                        "receiver": int(sl[0])
                    }

                    data_dict["recieved"].append(msg)
                    types.append(msg["type"])

        data.append(data_dict)

    types = list(set(types))

    msg_count = 0
    for node in data:
        msg_count += len(node["sent"])
        
    type_count = {type:0 for type in types}
    for node in data:
        for msg in node["sent"]:
            type_count[msg["type"]] += 1

    types, type_counts = zip(*list(type_count.items()))
    plt.bar(types, type_counts, color="r")
    plt.ylabel("Counts")
    plt.xlabel("Types")
    plt.title("TYPEWISE MSG DISTRIBUTION")
    plt.savefig(os.path.join(save_dir, f"typewise.png"))
    plt.close()

    sends_per_node = [len(node["sent"]) for node in data]
    recieves_per_node = [len(node["sent"]) for node in data]
    plt.bar(np.arange(len(data))-0.15, sends_per_node, color="r", label="SENT MSGS", width=0.3)
    plt.bar(np.arange(len(data))+0.15, sends_per_node, color="b", label="RECEIVED MSGS", width=0.3)
    plt.ylabel("Counts")
    plt.xlabel("Nodes")
    plt.title("MSGS SENT/RECIEVED PER NODE")
    plt.legend()
    plt.savefig(os.path.join(save_dir, f"nodewise.png"))
    plt.close()

    return msg_count, title



titles = []
msg_counts = []
for dir in os.listdir("./logs"):
    os.makedirs(f"./visualisations/{dir}", exist_ok=True)

    msg_count, title = process(os.path.join("./logs", dir),f"./visualisations/{dir}")
    titles.append(title)
    msg_counts.append(msg_count)

fig = plt.figure(figsize=(40,10))
ax = fig.add_subplot(1,1,1)
ax.bar(titles, msg_counts, color="g")
ax.set_title("TOTAL NUMBER OF MESSAGES GENERATED IN EACH EXPERIMENT")
fig.savefig("./visualisations/msg_counts.png")
plt.show()