# Esercizio 4 - Grafi Sparsi e Visita in Ampiezza

### Descrizione dell'implementazione

La struttura dati è un grafo sparso generico in cui: 
* i nodi sono memorizzati in una HashTable (dell'esercizio 2) che mappa `void* node → List* adjacency`, permettendo O(1) atteso per inserimento e ricerca dei nodi. 
* Ogni lista di adiacenza contiene elementi di tipo `ListNode` con campo `Edge { source, dest, label }`, così lo spazio occupato è proporzionale a N+M (nodi + archi), ideale per grafi sparsi. 

#### La libreria supporta: 
* **Grafi diretti / non diretti:** se `directed == 0`, `graph_add_edge()` inserisce **automaticamente** due archi, (a,b) e (b,a), per rappresentare un arco non diretto usando una struttura di base orientata. 

* **Grafi etichettati/non etichettati:** se `labelled == 1`, le label degli archi (campo `void* label`) devono essere non NULL ed è possibile recuperarle con `graph_get_label()`; se `labelled == 0`, le label devono essere sempre NULL. 

La genericità è ottenuta passando a `graph_create()` una funzione di confronto e una di hash. Nei test si usano versioni per interi, mentre nel programma principale si usano: 
* `compare_string` (wrapper di `strcmp()`)
* `hash_string` (`djb2`)
per gestire i nomi delle città letti dal CSV.

---

### Uso di `italian_dist_graph.csv` e BFS

Nel file `main_ex4` il grafo viene popolato leggendo il file `italian_dist_graph.csv`.

La funzione 
```c
int read_csv(const char *csv_name, Graph gr);
```

Esegue:
* Apertura del file `italian_dist_graph.csv` in lettura testo
* Per ogni riga, parsing dei 3 campi: `place1`, `place2`, `distance` tramite `strtok()` (delimitatore `,`)
* Inserimento dei nodi `place1` e `place2` se non ancora presenti, usando `graph_add_node()` e `strdup()` per avere una copia autonoma della stringa
* Conversione del campo `distance` in `float` tramite `string_to_float()`
* Creazione di una label `float*` e inserimento di **UN SOLO** arco etichettato da `place1` a `place2`

**IMPORTANTE**: La funzione `read_csv()` **non** inserisce manualmente l'arco inverso. Questo è gestito automaticamente dalla libreria.

Nel main: 
* Viene creato un grafo **etichettato e NON diretto** con `graph_create(1, 0, compare_string, hash_string)` 
  - `labelled = 1`: archi con label (distanza in metri)
  - `directed = 0`: **grafo non diretto** (duplicazione automatica degli archi)
* Si invoca `read_csv(argv[1], gr)` passando come primo parametro `italian_dist_graph.csv`
* Si controlla che la città di partenza (`argv[2]`, convertita in lowercase) sia presente con `graph_contains_node()`
* Si chiama: 
```c 
void **visited = breadth_first_visit(gr, argv[2], compare_string, hash_string);
```
che restituisce un array di nodi nell'ordine di visita in ampiezza, terminato da `NULL`
* L'array `visited` viene scritto riga per riga nel file di output (`argv[3]`)

Alla fine il programma libera: 
* `visited` 
* l'array degli archi restituito da `graph_get_edges` (compresi `source`, `dest` e `label` di ogni `Edge`)
* la struttura Graph con graph_free, evitando leak di memoria

---

### Gestione automatica grafi non diretti

**Da `graph.c`:**

```c
// In graph_add_edge():
if(!graph_is_directed(gr) && gr->compare(node1, node2) != 0) {
    Edge ed2 = {
        .source = (void *)node2,
        .dest = (void *)node1,
        .label = (void *)label
    };
    List *node2_adjacency = (List *)hash_table_get(gr->nodes, node2);
    list_push(node2_adjacency, &ed2);
}
```

Questa logica garantisce che:
1. **Se il grafo NON è diretto** (`directed == 0`)
2. **E i nodi sono diversi** (evita duplicazione self-loop)
3. **Allora viene inserito automaticamente l'arco inverso**

Quindi `read_csv` chiama `graph_add_edge` **una sola volta per riga**, e la libreria si occupa della simmetria.

---

### Implementazione BFS con HashTable

**Da `bfs.c`:**

```c
HashTable *visited_table = hash_table_create(compare, hash);
// ...
while(queue_tail != queue_head) {
    void *current = queue[queue_head++];
    visited[visited_size++] = current;
    
    void **neighbors = graph_get_neighbors(gr, current);
    for(int k = 0; neighbors[k] != NULL; k++) {
        if(!hash_table_contains_key(visited_table, neighbors[k])) {
            queue[queue_tail++] = neighbors[k];
            hash_table_put(visited_table, neighbors[k], NULL);
        }
    }
}
```

L'implementazione BFS **utilizza già una HashTable** (`visited_table`) per tracciare i nodi visitati, garantendo:
- **O(1) per controllare se un nodo è stato visitato**
- **Complessità totale O(N + M)** come da teoria standard

---

### Gestione della memoria
- **Allocazioni**: nodi e label creati solo quando letti da CSV (`strdup()` + `string_to_float()`)
- **Pulizia completa**: `graph_free()` + `main()` liberano:
    * Array `visited[]` 
    * Array `graph_get_edges()` (Edge + source/dest/label)
    * HashTable + tutte le List di adiacenza
- **Nessun leak**: ogni `strdup()` ha corrispondente `free()`

---

### Complessità rispettate

* **N**: Numero di nodi
* **M**: Numero di archi

| Operazione          | Complessità              | Motivazione sintetica                            |
| ------------------- | ------------------------ | ------------------------------------------------ |
| graph_add_node      | O(1)                     | inserimento in HashTable                         |
| graph_contains_node | O(1)                     | lookup in HashTable                              |
| graph_remove_node   | O(N)                     | scansione dei nodi per rimuovere archi incidenti |
| graph_num_nodes     | O(1)                     | wrapper di hash_table_size                       |
| graph_get_nodes     | O(N)                     | hash_table_keyset copia tutte le chiavi          |
| graph_add_edge      | O(1)                     | accesso HashTable + push in testa lista          |
| graph_contains_edge | O(1)*                    | scansione lista di adiacenza                     |
| graph_remove_edge   | O(1)*                    | rimozione dalla lista di adiacenza               |
| graph_num_edges     | O(N + M)                 | somma delle dimensioni di tutte le liste         |
| graph_get_edges     | O(N + M)                 | visita di tutte le liste e copia delle Edge      |
| graph_get_label     | O(1)*                    | scansione lista di adiacenza                     |
| graph_get_neighbors | O(1)*                    | copia dei puntatori dalla lista                  |
| graph_num_neighbors | O(1)                     | lettura del campo size della lista               | 
| **breadth_first_visit** | **O(N + M)**         | **HashTable per visitati + coda**                |

**\* Note**:
- Per operazioni che scorrono liste di adiacenza si può assumere tempo costante di esecuzione, dato che per ogni nodo vi sono 2/3 archi.

la **BFS** in breadth_first_visit:
* visita ogni nodo al massimo una volta. 
* usa `graph_get_neighbors()` per ottenere i vicini
* quindi ha complessità teorica O(N + M), come l’algoritmo standard di visita in ampiezza.

---

### Risultati sperimentali 

Esecuzione su `italian_dist_graph.csv` (~19000 nodi, ~48000 archi):

| Città partenza | Tempo BFS (s) |
|----------------|---------------|
| Torino         | 0.014971      |
| Milano         | 0.018464      |
| Roma           | 0.016025      |

**Nota**: I tempi confermano l'ordine di grandezza atteso per O(N + M) su questo dataset.

---

## Analisi critica

### Coerenza con la teoria degli algoritmi

L'uso della **HashTable** dell'esercizio 2 permette di rispettare la complessità **O(1) attesa** per operazioni locali sui nodi e, combinata con la **HashTable in BFS**, garantisce **O(N + M)** per la visita in ampiezza.

### Scelte implementative

1. **HashTable + liste di adiacenza**: Particolarmente adatta a grafi sparsi come quello delle distanze tra città: M < N² (48000 < 361.000.000), quindi una matrice di adiacenza sarebbe inefficiente sia in **spazio O(N²)** che in **tempo**.

2. **Gestione automatica archi non diretti**: `graph_add_edge` inserisce automaticamente l'arco inverso quando `directed=0`, evitando duplicazioni manuali nel codice client e riducendo possibilità di errori.

3. **BFS ottimizzata**: L'uso di `visited_table` (HashTable) per tracciare i nodi visitati garantisce O(1) per i controlli di appartenenza, rispettando la complessità teorica O(N + M).