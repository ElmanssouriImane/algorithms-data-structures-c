# Esercizio 3 - Heap Ternario 

## Descrizione dell'implementazione

La **PriorityQueue** implementa un heap ternario (ogni nodo ha 3 figli) con HashTable ausiliaria per il tracking delle posizioni:
```c
typedef struct PriorityQueue {
    void **data;           // array heap (max-heap: priorità decrescente)
    HashTable *pos_table;  // elemento → indice heap (O(1) contains/remove)
    intptr_t capacity, size;
    int (*compare)(...);   // priority DESC + start ASC
} PriorityQueue;
```

### Struttura heap ternario

In un heap ternario ogni nodo all'indice `i` ha:
- **Padre**: `(i-1)/3`
- **Figli**: `3i+1`, `3i+2`, `3i+3`

La proprietà di max-heap garantisce che ogni nodo abbia priorità maggiore o uguale ai suoi tre figli. Questo permette di mantenere il massimo in radice (`data[0]`) e accedervi in tempo O(1).

### Ruolo della HashTable

La `pos_table` mappa ogni elemento al suo indice corrente nell'array `data`. Questa struttura ausiliaria è fondamentale per ottenere le complessità richieste:
- `priority_queue_contains`: O(1) invece di O(N) grazie al lookup diretto nella hash table
- `priority_queue_remove`: O(log₃ N) invece di O(N) per trovare l'elemento, perché la ricerca dell'elemento diventa O(1)

Senza questa struttura ausiliaria, le operazioni `contains` e `remove` richiederebbero una scansione lineare dell'array, violando i requisiti di complessità della consegna.

## Complessità rispettate

| Operazione              | Complessità | Implementazione                    |
|-------------------------|-------------|------------------------------------|
| `priority_queue_create` | O(1)        | Allocazione strutture              |
| `priority_queue_push`   | O(log₃ N)   | Inserimento + heapify_up ternario  |
| `priority_queue_pop`    | O(log₃ N)   | Rimozione radice + heapify_down    |
| `priority_queue_top`    | O(1)        | Accesso a `data[0]`                |
| `priority_queue_contains`| O(1)       | Lookup in `hash_table`             |
| `priority_queue_remove` | O(log₃ N)   | Lookup O(1) + swap + heapify       |
| `priority_queue_size`   | O(1)        | Lettura campo `size`               |

**Nota**: La complessità O(log₃ N) dell'heap ternario corrisponde a circa 0.63 × log₂(N), quindi la profondità è minore rispetto a un heap binario. Tuttavia, ogni operazione di heapify richiede fino a 3 confronti per nodo invece di 2.

## Scheduling non-preemptive
Il programma `main_ex3 <input.csv> <output.csv>` legge i task da file e simula uno scheduler con le seguenti regole:

1. **Inizializzazione**: L'esecutore è libero al tempo t=0
2. **Arrivo task**: I task con Start ≤ t vengono inseriti nella PriorityQueue
3. **Selezione**: Viene estratto il task con priorità massima (o start time minore in caso di parità)
4. **Esecuzione non-preemptive**: Il task selezionato viene eseguito completamente, avanzando il tempo di Length unità
5. **Gestione idle**: Se la coda è vuota e ci sono task futuri, il tempo avanza di 1 unità (t++)
6. **Output**: Per ogni task eseguito viene scritto `ID,StartTime,EndTime` nel file di output

### Funzione di confronto

La `compare_task` implementa la logica richiesta:
```c
int compare_task(const void *a, const void *b) {
    Task *ta = (Task*)a;
    Task *tb = (Task*)b;
    
    // Confronto per HashTable: se ID uguali → 0
    if (ta->id == tb->id) return 0;
    
    // Altrimenti ordina per priorità DECRESCENTE (max-heap)
    if (ta->priority != tb->priority) 
        return ta->priority - tb->priority;
    
    // Tie-break: start time CRESCENTE
    return tb->start - ta->start;
}
```

Questa funzione è compatibile sia con la HashTable (controllo di uguaglianza basato su ID) che con la PriorityQueue (ordinamento per priorità decrescente e start crescente).

## Risultati sperimentali

Esecuzione su `tasks.csv` (1.000.000 task):

| Test                      | Tempo (s) | Note                               |
|---------------------------|-----------|-------------------------------------|
| `main_ex3 tasks.csv out`  | 0.638508     | Scheduling completo 1M task        |


### Analisi delle prestazioni
I tempi sono coerenti con le aspettative teoriche:

* Ogni task richiede 1 push + 1 pop → 2N × O(log₃ N) ≈ O(N log N)
* Con N=1.000.000, log₃(1000000) ≈ 12.6 livelli di profondità massima nell'heap
* 0.64 secondi per 2.000.000 operazioni logaritmiche dimostra un'eccellente performance pratica

La presenza di operazioni `contains` e `remove` occasionali (se richieste) non impatta significativamente il tempo totale grazie alla complessità O(1) e O(log₃ N) rispettivamente.

## Analisi critica

### Coerenza con la teoria

L'implementazione rispetta tutte le complessità richieste grazie all'uso combinato di:
1. **Heap ternario**: Operazioni logaritmiche per push/pop
2. **HashTable ausiliaria**: Accesso O(1) per contains/remove

Questa combinazione è essenziale: senza la HashTable, le operazioni `contains` e `remove` richiederebbero una scansione lineare dell'array, violando i requisiti di complessità.

### Scelte implementative

L'utilizzo del tipo `intptr_t`, definito in `stdint.h`, per i campi `capacity` e `size` della struct `PriorityQueue` garantisce compatibilità tra diverse architetture target (32 o 64 bit) e permette di evitare eventuali overflow. Questa scelta è particolarmente importante perché la `HashTable` associata memorizza gli indici come puntatori (tramite cast a `void *`): l'uso di `intptr_t` assicura che i valori degli indici possano essere convertiti in puntatori e viceversa senza perdita di informazioni.

#### Heap ternario vs binario
**Vantaggi dell'heap ternario**:
* Albero meno profondo: log₃ N < log₂ N (circa 37% in meno di livelli)
* Meno operazioni di risalita/discesa nei casi medi
* Migliore località della cache nei primi livelli

**Svantaggi**:
* Fino a 3 confronti per nodo invece di 2 durante heapify_down
* Calcolo degli indici leggermente più complesso (moltiplicazione per 3)

**Conclusione**: Le prestazioni pratiche sono comparabili, con un leggero vantaggio del ternario su dataset grandi dove la riduzione della profondità compensa i confronti extra.

#### Resize dinamico:
L'array dell'heap utilizza un meccanismo di crescita dinamica:

* La capacità raddoppia quando l'array è pieno: new_capacity = capacity × 2
* Complessità ammortizzata O(1) per push: sebbene il resize singolo sia O(N), avviene così raramente (dopo 2^k inserimenti) che il costo medio per inserimento rimane costante
* Controllo overflow: if(new_capacity < pq->capacity) previene integer overflow su sistemi a 32/64 bit

#### Genericità:
La struttura è completamente generica grazie a:

* `void *` per memorizzare elementi di qualsiasi tipo
* Funzioni compare e hash passate alla creazione
* Nessuna dipendenza dal tipo Task nella logica della priority queue

Questo permette di riutilizzare la stessa implementazione per diversi tipi di dati senza modifiche.

### Gestione del main:
Il `main_ex3` implementa una gestione robusta degli errori:

* Controllo esito di tutte le `fopen()` con messaggi di errore descrittivi
* In caso di fallimento: chiusura dei file già aperti, deallocazione della priority queue, terminazione con `return 1`
* In caso di successo: esecuzione scheduling, pulizia completa delle risorse, terminazione con `return 0`


#### Correttezza dello scheduling

Lo scheduler implementato rispetta fedelmente le specifiche:

* Non-preemptive: ogni task viene eseguito completamente prima di selezionarne un altro
* Priorità massima: tra i task già arrivati viene sempre scelto quello con priorità maggiore
* Tie-breaking corretto: a parità di priorità, viene preferito il task con start time minore
* Gestione idle: se la coda è vuota ma ci sono task futuri, il tempo avanza incrementalmente
* Output consistente: ogni riga contiene ID, tempo di inizio effettivo e tempo di fine

La correttezza è garantita dalla proprietà di max-heap e dalla funzione `compare_task()`.

## Gestione della memoria
L'implementazione garantisce l'assenza di memory leak attraverso:

* Task: Allocati dinamicamente durante la lettura del CSV, deallocati dopo l'esecuzione
* Array heap: Ridimensionato con realloc quando necessario, liberato in priority_queue_free
* HashTable: Completamente deallocata in priority_queue_free, incluse tutte le entry
* Invariante: Ogni allocazione ha una corrispondente deallocazione

La funzione execute_tasks garantisce che ogni task inserito nella coda venga eventualmente estratto e deallocato, anche in caso di terminazione anticipata.

## Conclusioni
L'implementazione proposta combina heap ternario e HashTable ausiliaria per rispettare tutte le complessità richieste dalla consegna. Il tempo di esecuzione di 0.64 secondi per processare 1.000.000 di task dimostra l'eccellente efficienza pratica della struttura, pienamente in linea con le aspettative teoriche O(N log N).

L'approccio adottato privilegia:
* Efficienza: Operazioni logaritmiche per push/pop, costanti per contains
* Correttezza: Invarianti dell'heap mantenute, consistenza tra heap e hash table
* Genericità: Riutilizzabile con qualsiasi tipo di dato tramite funzioni compare/hash
* Scalabilità: Resize dinamico con gestione dell'overflow
* Robustezza: Gestione completa degli errori e della memoria