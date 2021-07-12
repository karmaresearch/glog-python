import glog
import json
from embtopk.embtopktable import EmbTopKEDBTable
from kge.model import KgeModel
from kge.util.io import load_checkpoint

edb = glog.EDBLayer("edb.conf")
glog.set_logging_level(0)

# E is a new relation that maps to the embeddings
model_path = "/Users/jacopo/Desktop/probabilistic-glog/prob-tgs/embeddings/models/lubm_1_transe/checkpoint_best.pt"
print("Loading the model ...")
checkpoint = load_checkpoint(model_path)
model = KgeModel.create_from(checkpoint)
print("done.")

E = EmbTopKEDBTable("E", "<http://www.lehigh.edu/~zhp2/2004/0401/univ-bench.owl#memberOf>", 10, edb, model) #topk=10
edb.add_source("E", E)

print("Loading the program ...")
program = glog.Program(edb)
# Get the top k completions for the query <GraduateStudent97, memberOf, ?>
resp = program.add_rule("Q(X,S) :- E(<http://www.Department10.University0.edu/GraduateStudent97>,X,S)")
if type(resp) != int:
    raise Exception(resp)

# Compute the model
chaseProcedure = "probtgchase"
typeProv = "FULLPROV"
r = glog.Reasoner(chaseProcedure, edb, program, typeProv=typeProv, edbCheck=False,
        queryCont=False)
print("Reasoning ... ")
stats = r.create_model(0)
print("Statistics", stats)

tg = r.get_TG()
q = glog.Querier(tg)
nodes = q.get_node_details_predicate('Q')
nodes = json.loads(nodes)
for node in nodes:
    facts = q.get_facts_in_TG_node(int(node['id']))
    facts = json.loads(facts)
    for fact in facts:
        print(fact)


