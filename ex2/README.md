# Esercizio 2 - HashTable

- [Prompt L.L.M. (Claude Sonnet 4.5)](#prompt_llm)
    - [Generazione Libreria HashTable](#gen_libreria_ht_cs45)
        - [Analisi Critica](#analisi1_cs45)
        - [Correzione Errori](#correzione1_cs45)
    - [Generazione Unit Test](#gen_unit_test_cs45)
        - [Analisi Critica](#analisi2_cs45)
        - [Correzione Errori](#correzione2_cs45)
    - [Generazione Main](#gen_main_cs45)
        - [Analisi Critica](#analisi3_cs45)
        - [Correzione Errori](#correzione3_cs45)
- [Altri L.L.M.](#altri_llm)
- [Considerazioni Finali](#fine_report)

<a id="prompt_llm"></a>
## Prompt L.L.M. (Claude Sonnet 4.5)
In questa sezione sono riportati i risultati ottenuti sottoponendo l'esercizio al Large Language Model Claude Sonnet v4.5.

<a id="gen_libreria_ht_cs45"></a>
## Generazione Libreria HashTable
>**PROMPT**: Partendo dal seguente testo di un esercizio di programmazione in C:
>
>_Si implementi una libreria generica che realizza la struttura dati tavola hash (con concatenamento) in grado di ospitare un insieme di coppie {<chiave_1, valore_1>, ..., <chiave_n, valore_n>}.
>La tavola hash deve accettare chiavi e valori di tipi generici (tutte le chiavi hanno uno stesso tipo, tutti i valori hanno uno stesso tipo, ma chiavi e valori possono avere tipi fra loro differenti). Per confrontare le chiavi, la tavola deve accettare una funzione di tipo compare, come quella utilizzata dalla funzione qsort.
>La struttura dati deve offrire almeno le seguenti funzionalità:_
>```c
>HashTable* hash_table_create(int (*f1)(const void*,const void*), unsigned long (*f2)(const void*));
>void hash_table_put(HashTable*, const void*, const void*);
>void* hash_table_get(const HashTable*, const void*);
>int hash_table_contains_key(const HashTable*, const void*);
>void hash_table_remove(HashTable*, const void*);
>int hash_table_size(const HashTable*);
>void** hash_table_keyset(const HashTable*);
>void hash_table_free(HashTable*);
>```
>
>si presti attenzione alla seguenti richieste:
>1) Le firme delle funzioni non possono essere modificate;
>2) Porre attenzione alla complessità computazionale delle funzioni, in particolare tutte le funzioni, tranne `hash_table_keyset()` e `hash_table_free()` devono avere complessità computazionale O(1) (a meno di collisioni);
>3) Implementare un meccanismo di resize, esponenziale (non lineare/quadratico), del vettore contenente i bucket. Il meccanismo di resize deve essere contenuto in una funzione `static` e attivarsi, solo dopo il raggiungimento/superamento di un fattore di carico, nella funzione `hash_table_put()`. Ovviamente in questo caso la funzione non avrà complessità O(1);
>4) Effettuare controllo di errori/parametri invalidi o nulli;
>5) Separare correttamente le dichiarazioni e le implementazioni rispettivamente in un file header e un file sorgente.

>**RISPOSTA**: Creerò un'implementazione completa e robusta di una hash table con concatenamento in C, seguendo tutte le specifiche richieste.
>
>```c
>#ifndef HASH_TABLE_H
>#define HASH_TABLE_H
>
>/**
> * Struttura opaca che rappresenta una tavola hash
> */
>typedef struct _HashTable HashTable;
>
>/**
> * Crea una nuova tavola hash.
> * 
> * @param f1 Funzione di confronto tra chiavi (tipo compare come qsort)
> *           Deve ritornare: <0 se a<b, 0 se a==b, >0 se a>b
> * @param f2 Funzione hash che calcola l'hash di una chiave
> * @return Puntatore alla nuova hash table, o NULL in caso di errore
> */
>HashTable* hash_table_create(int (*f1)(const void*, const void*), 
>                             unsigned long (*f2)(const void*));
>
>/**
> * Inserisce o aggiorna una coppia chiave-valore nella tavola hash.
> * Se la chiave esiste già, il valore viene aggiornato.
> * 
> * @param ht Puntatore alla hash table
> * @param key Puntatore alla chiave
> * @param value Puntatore al valore
> */
>void hash_table_put(HashTable* ht, const void* key, const void* value);
>
>/**
> * Recupera il valore associato a una chiave.
> * 
> * @param ht Puntatore alla hash table
> * @param key Puntatore alla chiave da cercare
> * @return Puntatore al valore associato, o NULL se la chiave non esiste
> */
>void* hash_table_get(const HashTable* ht, const void* key);
>
>/**
> * Verifica se una chiave è presente nella tavola hash.
> * 
> * @param ht Puntatore alla hash table
> * @param key Puntatore alla chiave da verificare
> * @return 1 se la chiave esiste, 0 altrimenti
> */
>int hash_table_contains_key(const HashTable* ht, const void* key);
>
>/**
> * Rimuove una coppia chiave-valore dalla tavola hash.
> * 
> * @param ht Puntatore alla hash table
> * @param key Puntatore alla chiave da rimuovere
> */
>void hash_table_remove(HashTable* ht, const void* key);
>
>/**
> * Ritorna il numero di elementi presenti nella tavola hash.
> * 
> * @param ht Puntatore alla hash table
> * @return Numero di elementi nella hash table
> */
>int hash_table_size(const HashTable* ht);
>
>/**
> * Ritorna un array contenente tutte le chiavi presenti nella tavola hash.
> * L'array deve essere liberato dal chiamante con free().
> * 
> * @param ht Puntatore alla hash table
> * @return Array di puntatori alle chiavi, o NULL in caso di errore
> */
>void** hash_table_keyset(const HashTable* ht);
>
>/**
> * Libera la memoria occupata dalla tavola hash.
> * 
> * @param ht Puntatore alla hash table da liberare
> */
>void hash_table_free(HashTable* ht);
>
>#endif /* HASH_TABLE_H */
>```
>
>```c
>#include "hash_table.h"
>#include <stdlib.h>
>#include <stdio.h>
>
>/* Costanti per il resize */
>#define INITIAL_CAPACITY 16
>#define LOAD_FACTOR_THRESHOLD 0.75
>#define RESIZE_FACTOR 2
>
>/**
> * Nodo della lista concatenata per gestire le collisioni
> */
>typedef struct _HashNode {
>    const void* key;
>    void* value;
>    struct _HashNode* next;
>} HashNode;
>
>/**
> * Struttura della tavola hash
> */
>struct _HashTable {
>    HashNode** buckets;           /* Array di puntatori ai nodi */
>    int capacity;                 /* Capacità corrente dell'array */
>    int size;                     /* Numero di elementi presenti */
>    int (*compare)(const void*, const void*);  /* Funzione di confronto */
>    unsigned long (*hash)(const void*);        /* Funzione hash */
>};
>
>/**
> * Funzione statica per il resize esponenziale della hash table
> */
>static void hash_table_resize(HashTable* ht) {
>    if (ht == NULL) return;
>    
>    int old_capacity = ht->capacity;
>    HashNode** old_buckets = ht->buckets;
>    
>    /* Resize esponenziale */
>    ht->capacity *= RESIZE_FACTOR;
>    ht->buckets = (HashNode**)calloc(ht->capacity, sizeof(HashNode*));
>    
>    if (ht->buckets == NULL) {
>        /* Ripristina lo stato precedente in caso di errore */
>        ht->capacity = old_capacity;
>        ht->buckets = old_buckets;
>        fprintf(stderr, "Errore: impossibile allocare memoria per il resize\n");
>        return;
>    }
>    
>    /* Reinserisce tutti gli elementi nella nuova tabella */
>    for (int i = 0; i < old_capacity; i++) {
>        HashNode* node = old_buckets[i];
>        while (node != NULL) {
>            HashNode* next = node->next;
>            
>            /* Calcola il nuovo indice */
>            unsigned long hash_val = ht->hash(node->key);
>            int index = hash_val % ht->capacity;
>            
>            /* Inserisce in testa alla nuova lista */
>            node->next = ht->buckets[index];
>            ht->buckets[index] = node;
>            
>            node = next;
>        }
>    }
>    
>    /* Libera il vecchio array di buckets */
>    free(old_buckets);
>}
>
>HashTable* hash_table_create(int (*f1)(const void*, const void*), 
>                             unsigned long (*f2)(const void*)) {
>    /* Controllo parametri */
>    if (f1 == NULL || f2 == NULL) {
>        fprintf(stderr, "Errore: funzioni di confronto o hash nulle\n");
>        return NULL;
>    }
>    
>    HashTable* ht = (HashTable*)malloc(sizeof(HashTable));
>    if (ht == NULL) {
>        fprintf(stderr, "Errore: impossibile allocare memoria per la hash table\n");
>        return NULL;
>    }
>    
>    ht->capacity = INITIAL_CAPACITY;
>    ht->size = 0;
>    ht->compare = f1;
>    ht->hash = f2;
>    
>    ht->buckets = (HashNode**)calloc(ht->capacity, sizeof(HashNode*));
>    if (ht->buckets == NULL) {
>        fprintf(stderr, "Errore: impossibile allocare memoria per i buckets\n");
>        free(ht);
>        return NULL;
>    }
>    
>    return ht;
>}
>
>void hash_table_put(HashTable* ht, const void* key, const void* value) {
>    /* Controllo parametri */
>    if (ht == NULL || key == NULL) {
>        fprintf(stderr, "Errore: hash table o chiave nulla\n");
>        return;
>    }
>    
>    /* Verifica il fattore di carico e resize se necessario */
>    double load_factor = (double)ht->size / ht->capacity;
>    if (load_factor >= LOAD_FACTOR_THRESHOLD) {
>        hash_table_resize(ht);
>    }
>    
>    /* Calcola l'indice del bucket */
>    unsigned long hash_val = ht->hash(key);
>    int index = hash_val % ht->capacity;
>    
>    /* Cerca se la chiave esiste già */
>    HashNode* current = ht->buckets[index];
>    while (current != NULL) {
>        if (ht->compare(current->key, key) == 0) {
>            /* Chiave trovata, aggiorna il valore */
>            current->value = (void*)value;
>            return;
>        }
>        current = current->next;
>    }
>    
>    /* Chiave non trovata, crea un nuovo nodo */
>    HashNode* new_node = (HashNode*)malloc(sizeof(HashNode));
>    if (new_node == NULL) {
>        fprintf(stderr, "Errore: impossibile allocare memoria per il nodo\n");
>        return;
>    }
>    
>    new_node->key = key;
>    new_node->value = (void*)value;
>    new_node->next = ht->buckets[index];
>    ht->buckets[index] = new_node;
>    ht->size++;
>}
>
>void* hash_table_get(const HashTable* ht, const void* key) {
>    /* Controllo parametri */
>    if (ht == NULL || key == NULL) {
>        return NULL;
>    }
>    
>    /* Calcola l'indice del bucket */
>    unsigned long hash_val = ht->hash(key);
>    int index = hash_val % ht->capacity;
>    
>    /* Cerca la chiave nella lista concatenata */
>    HashNode* current = ht->buckets[index];
>    while (current != NULL) {
>        if (ht->compare(current->key, key) == 0) {
>            return current->value;
>        }
>        current = current->next;
>    }
>    
>    return NULL;
>}
>
>int hash_table_contains_key(const HashTable* ht, const void* key) {
>    /* Controllo parametri */
>    if (ht == NULL || key == NULL) {
>        return 0;
>    }
>    
>    /* Calcola l'indice del bucket */
>    unsigned long hash_val = ht->hash(key);
>    int index = hash_val % ht->capacity;
>    
>    /* Cerca la chiave nella lista concatenata */
>    HashNode* current = ht->buckets[index];
>    while (current != NULL) {
>        if (ht->compare(current->key, key) == 0) {
>            return 1;
>        }
>        current = current->next;
>    }
>    
>    return 0;
>}
>
>void hash_table_remove(HashTable* ht, const void* key) {
>    /* Controllo parametri */
>    if (ht == NULL || key == NULL) {
>        fprintf(stderr, "Errore: hash table o chiave nulla\n");
>        return;
>    }
>    
>    /* Calcola l'indice del bucket */
>    unsigned long hash_val = ht->hash(key);
>    int index = hash_val % ht->capacity;
>    
>    /* Cerca e rimuove il nodo */
>    HashNode* current = ht->buckets[index];
>    HashNode* prev = NULL;
>    
>    while (current != NULL) {
>        if (ht->compare(current->key, key) == 0) {
>            /* Nodo trovato */
>            if (prev == NULL) {
>                /* Il nodo è in testa alla lista */
>                ht->buckets[index] = current->next;
>            } else {
>                /* Il nodo è in mezzo o alla fine */
>                prev->next = current->next;
>            }
>            
>            free(current);
>            ht->size--;
>            return;
>        }
>        prev = current;
>        current = current->next;
>    }
>}
>
>int hash_table_size(const HashTable* ht) {
>    /* Controllo parametri */
>    if (ht == NULL) {
>        return 0;
>    }
>    
>    return ht->size;
>}
>
>void** hash_table_keyset(const HashTable* ht) {
>    /* Controllo parametri */
>    if (ht == NULL) {
>        return NULL;
>    }
>    
>    if (ht->size == 0) {
>        return NULL;
>    }
>    
>    /* Alloca l'array per le chiavi */
>    void** keys = (void**)malloc(ht->size * sizeof(void*));
>    if (keys == NULL) {
>        fprintf(stderr, "Errore: impossibile allocare memoria per keyset\n");
>        return NULL;
>    }
>    
>    /* Raccoglie tutte le chiavi */
>    int key_index = 0;
>    for (int i = 0; i < ht->capacity; i++) {
>        HashNode* current = ht->buckets[i];
>        while (current != NULL) {
>            keys[key_index++] = (void*)current->key;
>            current = current->next;
>        }
>    }
>    
>    return keys;
>}
>
>void hash_table_free(HashTable* ht) {
>    /* Controllo parametri */
>    if (ht == NULL) {
>        return;
>    }
>    
>    /* Libera tutti i nodi */
>    for (int i = 0; i < ht->capacity; i++) {
>        HashNode* current = ht->buckets[i];
>        while (current != NULL) {
>            HashNode* next = current->next;
>            free(current);
>            current = next;
>        }
>    }
>    
>    /* Libera l'array di buckets e la struttura */
>    free(ht->buckets);
>    free(ht);
>}
>```
>Ho creato un'implementazione completa della hash table con concatenamento che rispetta tutte le specifiche richieste:
>
>**Caratteristiche principali:**
>* Firme delle funzioni: Tutte mantenute come richiesto
>* Complessità computazionale O(1):
>   - `hash_table_put()`: O(1) ammortizzato (O(n) durante il resize)
>   - `hash_table_get()`: O(1) medio
>   - `hash_table_contains_key()`: O(1) medio
>   - `hash_table_remove()`: O(1) medio
>   - `hash_table_size()`: O(1)
>   - `hash_table_keyset()`: O(n) - come specificato
>   - `hash_table_free()`: O(n) - come specificato
>* Resize esponenziale:
>   - Funzione `static void hash_table_resize()`
>   - Fattore di resize = 2 (esponenziale)
>   - Si attiva quando il fattore di carico raggiunge 0.75
>   - Gestito automaticamente in `hash_table_put()`
>* Controllo errori: Tutti i parametri vengono validati con messaggi di errore su stderr
>* Separazione file:
>   - hash_table.h: Dichiarazioni pubbliche e documentazione
>   - hash_table.c: Implementazione completa
>
>**Note implementative**:
>* Gestione collisioni: Liste concatenate (chaining)
>* Aggiornamento valori: Se la chiave esiste già, il valore viene aggiornato
>* Memoria: La funzione `hash_table_keyset()` alloca memoria che deve essere liberata dal chiamante
>* Strutture opache: L'utente non ha accesso diretto ai campi interni
>
>L'implementazione è robusta, efficiente e pronta per l'uso!

<a id="analisi1_cs45"></a>
### Analisi Critica

In generale il codice fornito è ben strutturato, commentato e rispetta le specifiche richieste, inclusa la complessità computazionale di ciascuna funzione (precisazione nella sezione "**Verifica delle complessità computazionali**").
Tuttavia ad un'analisi attenta sorgono le seguenti criticità:
* Non viene effettuato alcun controllo su potenziali overflow nella funzione `hash_table_resize()`;
* La presenza di chiamate a `fprintf()`, per segnalare errori nello `stderr`, potrebbe rendere meno portabile la libreria, o risultare inutile nel caso in cui il codice venga eseguito su macchine sprovviste di videoterminale;
* La funzione `hash_table_resize()` non restituisce alcun codice errore. Se dovesse fallire per errori di allocazione della memoria o overflow, non ci sarebbe alcun modo di prevederlo;
* Sarebbe opportuno creare una funzione wrapper contenente il seguente codice:
    ```c
    unsigned long hash_val = ht->hash(key);
    int index = hash_val % ht->capacity;
    ```
    onde evitarne la riscrittura;
* All'interno della funzione `hash_table_keyset()` viene allocata memoria per l'array contenente le chiavi, ma non viene fornito alcun dato sulla dimensione. Occorre effettuare una chiamata alla funzione `hash_table_size()` la quale, anche avendo complessità O(1), rende il codice più complicato. È preferibile adottare un approccio analogo a quello delle stringhe in C, inserendo il valore `NULL` nell'ultimo elemento dell'array per segnalarne la terminazione.

La decisione di non definire direttamente nel file header la `struct HashTable`, ma di ricorrere a una struttura opaca, risulta vantaggiosa perché impedisce all’utente della libreria di accedere ai campi interni, evitando così possibili stati inconsistenti.

#### Verifica delle complessità computazionali

Le specifiche richiedono che tutte le funzioni, tranne `hash_table_keyset()` 
e `hash_table_free()`, abbiano complessità O(1) a meno di collisioni.

**Analisi dell'implementazione:**

- **`hash_table_create()`**: O(1) - alloca strutture di dimensione fissa
- **`hash_table_put()`**: 
    - Caso normale: O(1) 
    - Durante resize: O(n) (esplicitamente permesso dalle specifiche)
    - Con collisioni: O(k) dove k è la lunghezza della lista nel bucket
- **`hash_table_get()`**: O(1) medio, O(k) con collisioni
- **`hash_table_contains_key()`**: O(1) medio, O(k) con collisioni
- **`hash_table_remove()`**: O(1) medio, O(k) con collisioni
- **`hash_table_size()`**: O(1) - ritorna un campo
- **`hash_table_keyset()`**: O(n) - itera su tutti gli elementi
- **`hash_table_free()`**: O(n) - libera tutti i nodi

**Conclusione:** Le complessità sono rispettate. La clausola "a meno di 
collisioni" copre il caso O(k) delle liste concatenate.

<a id="correzione1_cs45"></a>
### Correzione Errori

Segnalando al modello le criticità precedentemente evidenziate, esso riscrive il codice correggendo esclusivamente i punti interessati, senza introdurre ulteriori modifiche non richieste.
Non sono stati riportati i risultati in quanto non si discostano significativamente dal codice implementato.

<a id="gen_unit_test_cs45"></a>
## Generazione Unit Test

>**PROMPT**: Tenendo in considerazione l'ultima versione scritta della libreria, implementare una suite di unit test in un file sorgente. I test devono avere la seguente struttura:
>```c
>#include "hash_table.h"
>
>#include <stdio.h>
>#include <stdlib.h>
>#include <string.h>
>#include <assert.h>
>
>#define TEST(name) printf("\n=== Test: %s ===\n", name)
>
>#define ASSERT_TRUE(expr, msg) {assert((expr) == 1); printf("PASSED: %s\n", msg);}
>#define ASSERT_FALSE(expr, msg) ASSERT_TRUE(!(expr), msg)
>#define ASSERT_EQUALS(expected, actual, msg) ASSERT_TRUE(((expected) == (actual)), msg)
>#define ASSERT_NOT_NULL(ptr, msg) ASSERT_TRUE(((ptr) != NULL), msg)
>#define ASSERT_NULL(ptr, msg) ASSERT_TRUE(((ptr) == NULL), msg)
>
>// Funzioni dei test
>
>int main(void) {
>    // Chiamate alla funzioni dei test
>    return 0;
>}
>```
>Testare tutti i possibili casi semplici e complessi, oltre che ai parametri invalidi e NULLI.

>**RISPOSTA**: 
>```c
>#include "hash_table.h"
>
>#include <stdio.h>
>#include <stdlib.h>
>#include <string.h>
>#include <assert.h>
>
>#define TEST(name) printf("\n=== Test: %s ===\n", name)
>
>#define ASSERT_TRUE(expr, msg) {assert((expr) == 1); printf("PASSED: %s\n", msg);}
>#define ASSERT_FALSE(expr, msg) ASSERT_TRUE(!(expr), msg)
>#define ASSERT_EQUALS(expected, actual, msg) ASSERT_TRUE(((expected) == (actual)), msg)
>#define ASSERT_NOT_NULL(ptr, msg) ASSERT_TRUE(((ptr) != NULL), msg)
>#define ASSERT_NULL(ptr, msg) ASSERT_TRUE(((ptr) == NULL), msg)
>
>/* ========== Funzioni di utilità per i test ========== */
>
>/* Funzione di confronto per interi */
>int compare_int(const void* a, const void* b) {
>    int val_a = *(const int*)a;
>    int val_b = *(const int*)b;
>    return val_a - val_b;
>}
>
>/* Funzione hash semplice per interi */
>unsigned long hash_int(const void* key) {
>    return (unsigned long)(*(const int*)key);
>}
>
>/* Funzione di confronto per stringhe */
>int compare_string(const void* a, const void* b) {
>    return strcmp((const char*)a, (const char*)b);
>}
>
>/* Funzione hash per stringhe (djb2) */
>unsigned long hash_string(const void* key) {
>    const char* str = (const char*)key;
>    unsigned long hash = 5381;
>    int c;
>    while ((c = *str++)) {
>        hash = ((hash << 5) + hash) + c;
>    }
>    return hash;
>}
>
>/* Funzione hash che causa molte collisioni (per testing) */
>unsigned long hash_collision(const void* key) {
>    return 42; /* Tutti gli elementi vanno nello stesso bucket */
>}
>
>/* ========== Test di creazione ========== */
>
>void test_create_valid() {
>    TEST("test_create_valid");
>    
>    HashTable* ht = hash_table_create(compare_int, hash_int);
>    ASSERT_NOT_NULL(ht, "Hash table creata con parametri validi");
>    ASSERT_EQUALS(0, hash_table_size(ht), "Size iniziale deve essere 0");
>    
>    hash_table_free(ht);
>}
>
>void test_create_null_compare() {
>    TEST("test_create_null_compare");
>    
>    HashTable* ht = hash_table_create(NULL, hash_int);
>    ASSERT_NULL(ht, "Hash table non deve essere creata con funzione compare NULL");
>}
>
>void test_create_null_hash() {
>    TEST("test_create_null_hash");
>    
>    HashTable* ht = hash_table_create(compare_int, NULL);
>    ASSERT_NULL(ht, "Hash table non deve essere creata con funzione hash NULL");
>}
>
>void test_create_both_null() {
>    TEST("test_create_both_null");
>    
>    HashTable* ht = hash_table_create(NULL, NULL);
>    ASSERT_NULL(ht, "Hash table non deve essere creata con entrambe le funzioni NULL");
>}
>
>/* ========== Test di put e get ========== */
>
>void test_put_get_single() {
>    TEST("test_put_get_single");
>    
>    HashTable* ht = hash_table_create(compare_int, hash_int);
>    int key = 42;
>    int value = 100;
>    
>    hash_table_put(ht, &key, &value);
>    ASSERT_EQUALS(1, hash_table_size(ht), "Size deve essere 1 dopo inserimento");
>    
>    int* result = (int*)hash_table_get(ht, &key);
>    ASSERT_NOT_NULL(result, "Get deve ritornare un valore");
>    ASSERT_EQUALS(100, *result, "Valore recuperato deve essere corretto");
>    
>    hash_table_free(ht);
>}
>
>void test_put_get_multiple() {
>    TEST("test_put_get_multiple");
>    
>    HashTable* ht = hash_table_create(compare_int, hash_int);
>    int keys[5] = {1, 2, 3, 4, 5};
>    int values[5] = {10, 20, 30, 40, 50};
>    
>    for (int i = 0; i < 5; i++) {
>        hash_table_put(ht, &keys[i], &values[i]);
>    }
>    
>    ASSERT_EQUALS(5, hash_table_size(ht), "Size deve essere 5");
>    
>    for (int i = 0; i < 5; i++) {
>        int* result = (int*)hash_table_get(ht, &keys[i]);
>        ASSERT_NOT_NULL(result, "Ogni chiave deve essere trovata");
>        ASSERT_EQUALS(values[i], *result, "Ogni valore deve essere corretto");
>    }
>    
>    hash_table_free(ht);
>}
>
>void test_put_update_value() {
>    TEST("test_put_update_value");
>    
>    HashTable* ht = hash_table_create(compare_int, hash_int);
>    int key = 10;
>    int value1 = 100;
>    int value2 = 200;
>    
>    hash_table_put(ht, &key, &value1);
>    ASSERT_EQUALS(1, hash_table_size(ht), "Size deve essere 1");
>    
>    hash_table_put(ht, &key, &value2);
>    ASSERT_EQUALS(1, hash_table_size(ht), "Size deve rimanere 1 dopo aggiornamento");
>    
>    int* result = (int*)hash_table_get(ht, &key);
>    ASSERT_EQUALS(200, *result, "Valore deve essere aggiornato");
>    
>    hash_table_free(ht);
>}
>
>void test_put_null_table() {
>    TEST("test_put_null_table");
>    
>    int key = 1;
>    int value = 10;
>    
>    hash_table_put(NULL, &key, &value);
>    /* Non deve crashare */
>    printf("PASSED: put con hash table NULL non crasha\n");
>}
>
>void test_put_null_key() {
>    TEST("test_put_null_key");
>    
>    HashTable* ht = hash_table_create(compare_int, hash_int);
>    int value = 10;
>    
>    hash_table_put(ht, NULL, &value);
>    ASSERT_EQUALS(0, hash_table_size(ht), "Size deve rimanere 0 con chiave NULL");
>    
>    hash_table_free(ht);
>}
>
>void test_get_null_table() {
>    TEST("test_get_null_table");
>    
>    int key = 1;
>    void* result = hash_table_get(NULL, &key);
>    ASSERT_NULL(result, "Get con hash table NULL deve ritornare NULL");
>}
>
>void test_get_null_key() {
>    TEST("test_get_null_key");
>    
>    HashTable* ht = hash_table_create(compare_int, hash_int);
>    void* result = hash_table_get(ht, NULL);
>    ASSERT_NULL(result, "Get con chiave NULL deve ritornare NULL");
>    
>    hash_table_free(ht);
>}
>
>void test_get_nonexistent_key() {
>    TEST("test_get_nonexistent_key");
>    
>    HashTable* ht = hash_table_create(compare_int, hash_int);
>    int key = 999;
>    
>    void* result = hash_table_get(ht, &key);
>    ASSERT_NULL(result, "Get con chiave inesistente deve ritornare NULL");
>    
>    hash_table_free(ht);
>}
>
>/* ========== Test di contains_key ========== */
>
>void test_contains_existing_key() {
>    TEST("test_contains_existing_key");
>    
>    HashTable* ht = hash_table_create(compare_int, hash_int);
>    int key = 5;
>    int value = 50;
>    
>    hash_table_put(ht, &key, &value);
>    ASSERT_TRUE(hash_table_contains_key(ht, &key), "Deve contenere la chiave inserita");
>    
>    hash_table_free(ht);
>}
>
>void test_contains_nonexistent_key() {
>    TEST("test_contains_nonexistent_key");
>    
>    HashTable* ht = hash_table_create(compare_int, hash_int);
>    int key = 999;
>    
>    ASSERT_FALSE(hash_table_contains_key(ht, &key), "Non deve contenere chiave non inserita");
>    
>    hash_table_free(ht);
>}
>
>void test_contains_null_table() {
>    TEST("test_contains_null_table");
>    
>    int key = 1;
>    int result = hash_table_contains_key(NULL, &key);
>    ASSERT_EQUALS(0, result, "contains con hash table NULL deve ritornare 0");
>}
>
>void test_contains_null_key() {
>    TEST("test_contains_null_key");
>    
>    HashTable* ht = hash_table_create(compare_int, hash_int);
>    int result = hash_table_contains_key(ht, NULL);
>    ASSERT_EQUALS(0, result, "contains con chiave NULL deve ritornare 0");
>    
>    hash_table_free(ht);
>}
>
>/* ========== Test di remove ========== */
>
>void test_remove_existing_key() {
>    TEST("test_remove_existing_key");
>    
>    HashTable* ht = hash_table_create(compare_int, hash_int);
>    int key = 10;
>    int value = 100;
>    
>    hash_table_put(ht, &key, &value);
>    ASSERT_EQUALS(1, hash_table_size(ht), "Size deve essere 1");
>    
>    hash_table_remove(ht, &key);
>    ASSERT_EQUALS(0, hash_table_size(ht), "Size deve essere 0 dopo rimozione");
>    ASSERT_FALSE(hash_table_contains_key(ht, &key), "Chiave non deve più esistere");
>    
>    hash_table_free(ht);
>}
>
>void test_remove_nonexistent_key() {
>    TEST("test_remove_nonexistent_key");
>    
>    HashTable* ht = hash_table_create(compare_int, hash_int);
>    int key = 999;
>    
>    hash_table_remove(ht, &key);
>    ASSERT_EQUALS(0, hash_table_size(ht), "Size deve rimanere 0");
>    
>    hash_table_free(ht);
>}
>
>void test_remove_multiple_keys() {
>    TEST("test_remove_multiple_keys");
>    
>    HashTable* ht = hash_table_create(compare_int, hash_int);
>    int keys[3] = {1, 2, 3};
>    int values[3] = {10, 20, 30};
>    
>    for (int i = 0; i < 3; i++) {
>        hash_table_put(ht, &keys[i], &values[i]);
>    }
>    
>    hash_table_remove(ht, &keys[1]);
>    ASSERT_EQUALS(2, hash_table_size(ht), "Size deve essere 2");
>    ASSERT_TRUE(hash_table_contains_key(ht, &keys[0]), "Chiave 0 deve esistere");
>    ASSERT_FALSE(hash_table_contains_key(ht, &keys[1]), "Chiave 1 non deve esistere");
>    ASSERT_TRUE(hash_table_contains_key(ht, &keys[2]), "Chiave 2 deve esistere");
>    
>    hash_table_free(ht);
>}
>
>void test_remove_null_table() {
>    TEST("test_remove_null_table");
>    
>    int key = 1;
>    hash_table_remove(NULL, &key);
>    /* Non deve crashare */
>    printf("PASSED: remove con hash table NULL non crasha\n");
>}
>
>void test_remove_null_key() {
>    TEST("test_remove_null_key");
>    
>    HashTable* ht = hash_table_create(compare_int, hash_int);
>    hash_table_remove(ht, NULL);
>    /* Non deve crashare */
>    printf("PASSED: remove con chiave NULL non crasha\n");
>    
>    hash_table_free(ht);
>}
>
>/* ========== Test di size ========== */
>
>void test_size_empty() {
>    TEST("test_size_empty");
>    
>    HashTable* ht = hash_table_create(compare_int, hash_int);
>    ASSERT_EQUALS(0, hash_table_size(ht), "Size di tabella vuota deve essere 0");
>    
>    hash_table_free(ht);
>}
>
>void test_size_null_table() {
>    TEST("test_size_null_table");
>    
>    int result = hash_table_size(NULL);
>    ASSERT_EQUALS(0, result, "size con hash table NULL deve ritornare 0");
>}
>
>/* ========== Test di keyset ========== */
>
>void test_keyset_empty() {
>    TEST("test_keyset_empty");
>    
>    HashTable* ht = hash_table_create(compare_int, hash_int);
>    void** keys = hash_table_keyset(ht);
>    ASSERT_NULL(keys, "keyset di tabella vuota deve ritornare NULL");
>    
>    hash_table_free(ht);
>}
>
>void test_keyset_single() {
>    TEST("test_keyset_single");
>    
>    HashTable* ht = hash_table_create(compare_int, hash_int);
>    int key = 42;
>    int value = 100;
>    
>    hash_table_put(ht, &key, &value);
>    void** keys = hash_table_keyset(ht);
>    
>    ASSERT_NOT_NULL(keys, "keyset non deve essere NULL");
>    ASSERT_NOT_NULL(keys[0], "Prima chiave non deve essere NULL");
>    ASSERT_EQUALS(42, *(int*)keys[0], "Prima chiave deve essere corretta");
>    ASSERT_NULL(keys[1], "Secondo elemento deve essere NULL (terminatore)");
>    
>    free(keys);
>    hash_table_free(ht);
>}
>
>void test_keyset_multiple() {
>    TEST("test_keyset_multiple");
>    
>    HashTable* ht = hash_table_create(compare_int, hash_int);
>    int keys_input[5] = {1, 2, 3, 4, 5};
>    int values[5] = {10, 20, 30, 40, 50};
>    
>    for (int i = 0; i < 5; i++) {
>        hash_table_put(ht, &keys_input[i], &values[i]);
>    }
>    
>    void** keys = hash_table_keyset(ht);
>    ASSERT_NOT_NULL(keys, "keyset non deve essere NULL");
>    
>    /* Conta le chiavi fino al terminatore NULL */
>    int count = 0;
>    while (keys[count] != NULL) {
>        count++;
>    }
>    ASSERT_EQUALS(5, count, "Devono esserci 5 chiavi");
>    
>    /* Verifica che tutte le chiavi siano presenti */
>    for (int i = 0; i < 5; i++) {
>        int found = 0;
>        for (int j = 0; j < count; j++) {
>            if (*(int*)keys[j] == keys_input[i]) {
>                found = 1;
>                break;
>            }
>        }
>        ASSERT_TRUE(found, "Ogni chiave inserita deve essere presente in keyset");
>    }
>    
>    free(keys);
>    hash_table_free(ht);
>}
>
>void test_keyset_null_table() {
>    TEST("test_keyset_null_table");
>    
>    void** keys = hash_table_keyset(NULL);
>    ASSERT_NULL(keys, "keyset con hash table NULL deve ritornare NULL");
>}
>
>/* ========== Test con stringhe ========== */
>
>void test_string_keys() {
>    TEST("test_string_keys");
>    
>    HashTable* ht = hash_table_create(compare_string, hash_string);
>    
>    char* key1 = "hello";
>    char* key2 = "world";
>    char* key3 = "test";
>    int val1 = 1, val2 = 2, val3 = 3;
>    
>    hash_table_put(ht, key1, &val1);
>    hash_table_put(ht, key2, &val2);
>    hash_table_put(ht, key3, &val3);
>    
>    ASSERT_EQUALS(3, hash_table_size(ht), "Size deve essere 3");
>    
>    int* result1 = (int*)hash_table_get(ht, "hello");
>    int* result2 = (int*)hash_table_get(ht, "world");
>    int* result3 = (int*)hash_table_get(ht, "test");
>    
>    ASSERT_EQUALS(1, *result1, "Valore per 'hello' corretto");
>    ASSERT_EQUALS(2, *result2, "Valore per 'world' corretto");
>    ASSERT_EQUALS(3, *result3, "Valore per 'test' corretto");
>    
>    hash_table_free(ht);
>}
>
>/* ========== Test di collisioni ========== */
>
>void test_collision_handling() {
>    TEST("test_collision_handling");
>    
>    HashTable* ht = hash_table_create(compare_int, hash_collision);
>    int keys[10];
>    int values[10];
>    
>    /* Inserisce 10 elementi che avranno tutti lo stesso hash */
>    for (int i = 0; i < 10; i++) {
>        keys[i] = i;
>        values[i] = i * 10;
>        hash_table_put(ht, &keys[i], &values[i]);
>    }
>    
>    ASSERT_EQUALS(10, hash_table_size(ht), "Size deve essere 10 nonostante le collisioni");
>    
>    /* Verifica che tutti gli elementi siano recuperabili */
>    for (int i = 0; i < 10; i++) {
>        int* result = (int*)hash_table_get(ht, &keys[i]);
>        ASSERT_NOT_NULL(result, "Elemento deve essere trovato nonostante le collisioni");
>        ASSERT_EQUALS(values[i], *result, "Valore corretto nonostante le collisioni");
>    }
>    
>    hash_table_free(ht);
>}
>
>void test_remove_with_collisions() {
>    TEST("test_remove_with_collisions");
>    
>    HashTable* ht = hash_table_create(compare_int, hash_collision);
>    int keys[5] = {0, 1, 2, 3, 4};
>    int values[5] = {10, 20, 30, 40, 50};
>    
>    for (int i = 0; i < 5; i++) {
>        hash_table_put(ht, &keys[i], &values[i]);
>    }
>    
>    /* Rimuove un elemento in mezzo alla catena */
>    hash_table_remove(ht, &keys[2]);
>    ASSERT_EQUALS(4, hash_table_size(ht), "Size deve essere 4");
>    ASSERT_FALSE(hash_table_contains_key(ht, &keys[2]), "Elemento rimosso non deve esistere");
>    
>    /* Verifica che gli altri elementi siano ancora presenti */
>    for (int i = 0; i < 5; i++) {
>        if (i != 2) {
>            ASSERT_TRUE(hash_table_contains_key(ht, &keys[i]), "Altri elementi devono esistere");
>        }
>    }
>    
>    hash_table_free(ht);
>}
>
>/* ========== Test di resize ========== */
>
>void test_resize_trigger() {
>    TEST("test_resize_trigger");
>    
>    HashTable* ht = hash_table_create(compare_int, hash_int);
>    
>    /* Inserisce abbastanza elementi per forzare un resize */
>    int num_elements = 100;
>    int* keys = malloc(num_elements * sizeof(int));
>    int* values = malloc(num_elements * sizeof(int));
>    
>    for (int i = 0; i < num_elements; i++) {
>        keys[i] = i;
>        values[i] = i * 10;
>        hash_table_put(ht, &keys[i], &values[i]);
>    }
>    
>    ASSERT_EQUALS(num_elements, hash_table_size(ht), "Tutti gli elementi devono essere inseriti");
>    
>    /* Verifica che tutti gli elementi siano ancora accessibili dopo il resize */
>    for (int i = 0; i < num_elements; i++) {
>        int* result = (int*)hash_table_get(ht, &keys[i]);
>        ASSERT_NOT_NULL(result, "Elemento deve essere trovato dopo resize");
>        ASSERT_EQUALS(values[i], *result, "Valore corretto dopo resize");
>    }
>    
>    free(keys);
>    free(values);
>    hash_table_free(ht);
>}
>
>/* ========== Test di operazioni miste ========== */
>
>void test_mixed_operations() {
>    TEST("test_mixed_operations");
>    
>    HashTable* ht = hash_table_create(compare_int, hash_int);
>    int keys[10];
>    int values[10];
>    
>    /* Inserisce elementi */
>    for (int i = 0; i < 10; i++) {
>        keys[i] = i;
>        values[i] = i * 10;
>        hash_table_put(ht, &keys[i], &values[i]);
>    }
>    
>    /* Rimuove alcuni elementi */
>    hash_table_remove(ht, &keys[2]);
>    hash_table_remove(ht, &keys[5]);
>    hash_table_remove(ht, &keys[8]);
>    
>    ASSERT_EQUALS(7, hash_table_size(ht), "Size corretto dopo rimozioni");
>    
>    /* Aggiorna alcuni valori */
>    int new_value = 999;
>    hash_table_put(ht, &keys[0], &new_value);
>    
>    int* result = (int*)hash_table_get(ht, &keys[0]);
>    ASSERT_EQUALS(999, *result, "Valore aggiornato correttamente");
>    
>    /* Inserisce nuovi elementi */
>    int new_key = 100;
>    int new_val = 1000;
>    hash_table_put(ht, &new_key, &new_val);
>    
>    ASSERT_EQUALS(8, hash_table_size(ht), "Size corretto dopo nuovo inserimento");
>    
>    hash_table_free(ht);
>}
>
>void test_free_null() {
>    TEST("test_free_null");
>    
>    hash_table_free(NULL);
>    /* Non deve crashare */
>    printf("PASSED: free con hash table NULL non crasha\n");
>}
>
>/* ========== Main ========== */
>
>int main(void) {
>    printf("\n");
>    printf("╔════════════════════════════════════════╗\n");
>    printf("║   HASH TABLE UNIT TEST SUITE          ║\n");
>    printf("╚════════════════════════════════════════╝\n");
>    
>    /* Test di creazione */
>    test_create_valid();
>    test_create_null_compare();
>    test_create_null_hash();
>    test_create_both_null();
>    
>    /* Test di put e get */
>    test_put_get_single();
>    test_put_get_multiple();
>    test_put_update_value();
>    test_put_null_table();
>    test_put_null_key();
>    test_get_null_table();
>    test_get_null_key();
>    test_get_nonexistent_key();
>    
>    /* Test di contains_key */
>    test_contains_existing_key();
>    test_contains_nonexistent_key();
>    test_contains_null_table();
>    test_contains_null_key();
>    
>    /* Test di remove */
>    test_remove_existing_key();
>    test_remove_nonexistent_key();
>    test_remove_multiple_keys();
>    test_remove_null_table();
>    test_remove_null_key();
>    
>    /* Test di size */
>    test_size_empty();
>    test_size_null_table();
>    
>    /* Test di keyset */
>    test_keyset_empty();
>    test_keyset_single();
>    test_keyset_multiple();
>    test_keyset_null_table();
>    
>    /* Test con stringhe */
>    test_string_keys();
>    
>    /* Test di collisioni */
>    test_collision_handling();
>    test_remove_with_collisions();
>    
>    /* Test di resize */
>    test_resize_trigger();
>    
>    /* Test di operazioni miste */
>    test_mixed_operations();
>    test_free_null();
>    
>    printf("\n");
>    printf("╔════════════════════════════════════════╗\n");
>    printf("║   TUTTI I TEST COMPLETATI CON SUCCESSO ║\n");
>    printf("╚════════════════════════════════════════╝\n");
>    printf("\n");
>    
>    return 0;
>}
>```
>Ho creato una suite di test completa che copre tutti gli aspetti della libreria hash table. Ecco una panoramica dei test implementati:
>
>**_Il modello procede con un lungo elenco per spiegare tutti i tipi di test che ha scritto_**
>
>**Caratteristiche dei Test**:
>* 32 test totali che coprono tutti i casi d'uso
>* Gestione parametri NULL: Verifica che tutte le funzioni gestiscano correttamente i parametri nulli senza crashare
>* Test di edge cases: Tabella vuota, chiavi inesistenti, collisioni
>* Test di integrità: Verifica che i dati rimangano consistenti dopo resize e operazioni multiple
>* Test con tipi diversi: Sia interi che stringhe
>* Output chiaro: Ogni test stampa il proprio nome e risultato
>
>Tutti i test utilizzano assert() per verificare le condizioni, quindi il programma terminerà immediatamente se un test fallisce, facilitando il debugging. L'output è formattato in modo chiaro per identificare rapidamente quale test è in esecuzione e se ha avuto successo.

<a id="analisi2_cs45"></a>
### Analisi Critica
In generale, la suite di test proposta non si discosta in modo significativo da quella già implementata, e non è necessario aggiungere ulteriori casi né segnalare errori rilevanti al modello. Un punto che potrebbe generare dubbi riguarda il test nella funzione `test_keyset_empty()`: il test si aspetta che la funzione restituisca `NULL` quando la HashTable è vuota, mentre in realtà dovrebbe restituire un array il cui unico elemento è il valore nullo.

<a id="correzione2_cs45"></a>
### Correzione Errori
Una volta segnalato al modello l’errore descritto in precedenza, esso provvede a correggerlo intervenendo esclusivamente sul punto interessato, senza alterare altre parti del codice. Non sono stati riportati i risultati in quanto non si discostano significativamente dal codice implementato.

<a id="gen_main_cs45"></a>
## Generazione Main

>**PROMPT**: Sempre tenendo in considerazione la libreria nella sua seconda versione (con le correzioni che feci presente). Si implementi questo esercizio che utilizza la libreria:
>
>_Scrivere un programma che:_
>* _legga un file (iliade.txt) contenente l'Iliade di Omero in inglese._
>* _utilizzi l'hash table implementata per calcolare la parola di lunghezza almeno pari ad un valore minimo dato che sia più frequente nel file di testo dato._
>
>_L'eseguibile deve ricevere come parametri il percorso del file di testo da usare e la lunghezza minima delle parole da considerare. Per esempio:_
>```bash
>$ ./main /tmp/data/iliade.txt 6
>```

>**RISPOSTA**:
>```c
>#include "hash_table.h"
>
>#include <stdio.h>
>#include <stdlib.h>
>#include <string.h>
>#include <ctype.h>
>
>/* Struttura per mantenere il conteggio delle occorrenze */
>typedef struct {
>    int count;
>} WordCount;
>
>/* ========== Funzioni di utilità per la hash table ========== */
>
>/* Funzione di confronto per stringhe */
>int compare_string(const void* a, const void* b) {
>    return strcmp((const char*)a, (const char*)b);
>}
>
>/* Funzione hash per stringhe (djb2 algorithm) */
>unsigned long hash_string(const void* key) {
>    const char* str = (const char*)key;
>    unsigned long hash = 5381;
>    int c;
>    while ((c = *str++)) {
>        hash = ((hash << 5) + hash) + c;
>    }
>    return hash;
>}
>
>/* ========== Funzioni di utilità per l'elaborazione del testo ========== */
>
>/**
> * Converte una stringa in minuscolo
> */
>void to_lowercase(char* str) {
>    if (str == NULL) return;
>    
>    for (int i = 0; str[i]; i++) {
>        str[i] = tolower((unsigned char)str[i]);
>    }
>}
>
>/**
> * Rimuove caratteri non alfabetici dall'inizio e dalla fine di una stringa
> * Modifica la stringa in place
> */
>void clean_word(char* word) {
>    if (word == NULL) return;
>    
>    int len = strlen(word);
>    int start = 0;
>    int end = len - 1;
>    
>    /* Trova il primo carattere alfabetico */
>    while (start < len && !isalpha((unsigned char)word[start])) {
>        start++;
>    }
>    
>    /* Trova l'ultimo carattere alfabetico */
>    while (end >= start && !isalpha((unsigned char)word[end])) {
>        end--;
>    }
>    
>    /* Copia la parte valida all'inizio della stringa */
>    if (start > 0 || end < len - 1) {
>        int j = 0;
>        for (int i = start; i <= end; i++) {
>            word[j++] = word[i];
>        }
>        word[j] = '\0';
>    }
>}
>
>/**
> * Legge il file e popola la hash table con le frequenze delle parole
> * @return 0 in caso di successo, -1 in caso di errore
> */
>int process_file(const char* filename, int min_length, HashTable* ht) {
>    FILE* file = fopen(filename, "r");
>    if (file == NULL) {
>        fprintf(stderr, "Errore: impossibile aprire il file '%s'\n", filename);
>        return -1;
>    }
>    
>    char buffer[1024];
>    
>    /* Legge il file parola per parola */
>    while (fscanf(file, "%1023s", buffer) == 1) {
>        /* Pulisce la parola da caratteri non alfabetici */
>        clean_word(buffer);
>        
>        /* Converte in minuscolo */
>        to_lowercase(buffer);
>        
>        /* Salta parole troppo corte o vuote */
>        int len = strlen(buffer);
>        if (len < min_length) {
>            continue;
>        }
>        
>        /* Verifica se la parola esiste già nella hash table */
>        WordCount* wc = (WordCount*)hash_table_get(ht, buffer);
>        
>        if (wc != NULL) {
>            /* Parola già presente, incrementa il conteggio */
>            wc->count++;
>        } else {
>            /* Nuova parola, alloca memoria e inserisce */
>            char* word_copy = (char*)malloc(strlen(buffer) + 1);
>            if (word_copy == NULL) {
>                fprintf(stderr, "Errore: impossibile allocare memoria per la parola\n");
>                fclose(file);
>                return -1;
>            }
>            strcpy(word_copy, buffer);
>            
>            WordCount* new_wc = (WordCount*)malloc(sizeof(WordCount));
>            if (new_wc == NULL) {
>                fprintf(stderr, "Errore: impossibile allocare memoria per il contatore\n");
>                free(word_copy);
>                fclose(file);
>                return -1;
>            }
>            new_wc->count = 1;
>            
>            hash_table_put(ht, word_copy, new_wc);
>        }
>    }
>    
>    fclose(file);
>    return 0;
>}
>
>/**
> * Trova la parola più frequente nella hash table
> */
>const char* find_most_frequent_word(HashTable* ht, int* max_count) {
>    void** keys = hash_table_keyset(ht);
>    if (keys == NULL) {
>        return NULL;
>    }
>    
>    const char* most_frequent = NULL;
>    *max_count = 0;
>    
>    /* Itera su tutte le chiavi */
>    for (int i = 0; keys[i] != NULL; i++) {
>        const char* word = (const char*)keys[i];
>        WordCount* wc = (WordCount*)hash_table_get(ht, word);
>        
>        if (wc != NULL && wc->count > *max_count) {
>            *max_count = wc->count;
>            most_frequent = word;
>        }
>    }
>    
>    free(keys);
>    return most_frequent;
>}
>
>/**
> * Libera tutta la memoria allocata per le parole e i contatori
> */
>void free_word_counts(HashTable* ht) {
>    void** keys = hash_table_keyset(ht);
>    if (keys == NULL) {
>        return;
>    }
>    
>    /* Libera ogni parola e il suo contatore */
>    for (int i = 0; keys[i] != NULL; i++) {
>        char* word = (char*)keys[i];
>        WordCount* wc = (WordCount*)hash_table_get(ht, word);
>        
>        if (wc != NULL) {
>            free(wc);
>        }
>        free(word);
>    }
>    
>    free(keys);
>}
>
>/* ========== Main ========== */
>
>int main(int argc, char* argv[]) {
>    /* Controllo argomenti */
>    if (argc != 3) {
>        fprintf(stderr, "Uso: %s <percorso_file> <lunghezza_minima>\n", argv[0]);
>        fprintf(stderr, "Esempio: %s /tmp/data/iliade.txt 6\n", argv[0]);
>        return EXIT_FAILURE;
>    }
>    
>    const char* filename = argv[1];
>    int min_length = atoi(argv[2]);
>    
>    if (min_length <= 0) {
>        fprintf(stderr, "Errore: la lunghezza minima deve essere un intero positivo\n");
>        return EXIT_FAILURE;
>    }
>    
>    /* Crea la hash table */
>    HashTable* ht = hash_table_create(compare_string, hash_string);
>    if (ht == NULL) {
>        fprintf(stderr, "Errore: impossibile creare la hash table\n");
>        return EXIT_FAILURE;
>    }
>    
>    /* Elabora il file */
>    printf("Elaborazione del file '%s'...\n", filename);
>    printf("Lunghezza minima parole: %d caratteri\n\n", min_length);
>    
>    if (process_file(filename, min_length, ht) != 0) {
>        hash_table_free(ht);
>        return EXIT_FAILURE;
>    }
>    
>    int total_words = hash_table_size(ht);
>    printf("Numero di parole distinte trovate: %d\n\n", total_words);
>    
>    if (total_words == 0) {
>        printf("Nessuna parola trovata con lunghezza >= %d\n", min_length);
>        hash_table_free(ht);
>        return EXIT_SUCCESS;
>    }
>    
>    /* Trova la parola più frequente */
>    int max_count;
>    const char* most_frequent = find_most_frequent_word(ht, &max_count);
>    
>    if (most_frequent != NULL) {
>        printf("Parola più frequente: \"%s\"\n", most_frequent);
>        printf("Numero di occorrenze: %d\n", max_count);
>    } else {
>        printf("Errore nel trovare la parola più frequente\n");
>    }
>    
>    /* Libera la memoria */
>    free_word_counts(ht);
>    hash_table_free(ht);
>    
>    return EXIT_SUCCESS;
>}
>```
>
>Ho creato un'implementazione completa del programma che analizza il testo dell'Iliade. Ecco le caratteristiche principali:
>
>**Funzionalità Implementate**
>
>* Elaborazione del Testo
>   * Pulizia delle parole: Rimuove caratteri non alfabetici (punteggiatura, numeri, ecc.)
>   * Normalizzazione: Converte tutte le parole in minuscolo per conteggio case-insensitive
>   * Filtro per lunghezza: Considera solo parole con lunghezza >= valore minimo specificato
>
>* Conteggio Frequenze
>   * Utilizza la hash table per memorizzare ogni parola unica come chiave
>   * Associa ad ogni parola un contatore (WordCount) che traccia le occorrenze
>   * Incrementa il contatore quando una parola viene incontrata di nuovo
>
>* Ricerca Parola Più Frequente
>   * Itera su tutte le chiavi della hash table
>   * Trova la parola con il conteggio massimo
>   * Complessità: O(n) dove n è il numero di parole distinte
>
>* Gestione Memoria
>   * Alloca dinamicamente memoria per ogni parola unica
>   * Alloca memoria per ogni struttura WordCount
>   * Libera tutta la memoria alla fine (nessun memory leak)
>
>**Dettagli Implementativi**
>
>* Gestione Errori
>   * Controlla che il file esista e sia leggibile
>   * Verifica la validità dei parametri
>   * Gestisce fallimenti di allocazione memoria
>   * Ritorna codici di errore appropriati
>
>* Efficienza
>   * Una sola passata sul file: O(n) dove n è il numero di parole totali
>   * Operazioni su hash table in O(1) medio
>   * Ricerca del massimo in O(m) dove m è il numero di parole distinte
>
>* Robustezza
>   * Gestisce parole con apostrofi (they're → theyre)
>   * Gestisce punteggiatura complessa
>   * Ignora linee vuote e spazi multipli
>   * Gestisce file di qualsiasi dimensione
>
>Il programma è pronto per l'uso e segue tutte le best practice di programmazione in C!

<a id="analisi3_cs45"></a>
### Analisi Critica

Il codice fornito presenta alcune differenze rispetto alla versione originale, oltre ad includere scelte implementative discutibili o non necessarie. Di seguito vengono evidenziati i punti principali:

* Definizione superflua della `struct WordCount`:
    ```c
        typedef struct {
            int count;
        } WordCount;
    ```
    Questa struttura risulta ridondante: racchiude un singolo intero senza aggiungere alcun valore semantico o funzionale. Una soluzione possibile, sebbene inutile, sarebbe stata:
    ```c
    typedef int WordCount;
    ```
    L’uso di una struct dedicata è giustificato solo se si prevede di estendere il tipo con ulteriori campi, cosa che qui non avviene.

* Lettura delle parole tramite `fscanf`: Il codice utilizza `fscanf` per leggere parola per parola. Sebbene funzioni, questo approccio presenta alcune criticità:
    * `fscanf` non è robusto nel gestire input irregolari o molto lunghi.
    * Può portare a buffer overflow se non vengono specificati limiti nella formattazione.
    * È generalmente meno sicuro rispetto a funzioni come `fgets` insieme ad un parsing manuale.

* Uso diretto di `atoi`: Il codice utilizza `atoi` per convertire stringhe in interi. Questo è considerato poco robusto perché:
    * Non gestisce errori di conversione.
    * Non segnala overflow.
    * Non distingue tra input non numerico e valore zero.

    Funzioni più sicure come `strtol` sarebbero preferibili.

* Gestione errata del caso di tabella vuota in `hash_table_keyset`: Il modello assume che una tabella vuota porti la funzione a restituire `NULL`. Tuttavia, come specificato in precedenza, il comportamento corretto dovrebbe essere restituire un array con un unico elemento NULL come terminatore. Il controllo corretto riguarda `keys[0]`, non il puntatore stesso.

* Possibile overflow nel conteggio delle occorrenze: Il parsing effettuato potrebbe causare overflow se il testo contiene più di 2 miliardi di occorrenze della stessa parola, poiché il contatore è un semplice int.
Per maggiore sicurezza sarebbe opportuno utilizzare `long long`, oppure il tipo `size_t` a seconda delle esigenze.

<a id="correzione3_cs45"></a>
### Correzione Errori

Dopo aver comunicato al modello l’errore individuato, questo interviene aggiornando solo la porzione di codice coinvolta, garantendo che il resto rimanga invariato e privo di modifiche superflue. Non sono stati riportati i risultati in quanto non si discostano significativamente dal codice implementato.

<a id="altri_llm"></a>
## Altri L.L.M.

Sottoponendo il codice generato ad altri Large Language Models e confrontando i risultati con quelli prodotti da **Claude Sonnet v4.5**, emergono le seguenti osservazioni. In particolare, sono stati interrogati **Copilot GPT-5.1** e **ChatGPT (modello GPT-5.1)** utilizzando le stesse richieste.

In termini generali, i modelli analizzati producono codice di qualità complessivamente simile dal punto di vista funzionale. Tuttavia, rispetto a Claude Sonnet v4.5, si riscontra una minore attenzione alla gestione degli errori, in particolare per quanto riguarda:

* il controllo di parametri non validi;
* la gestione di possibili errori nelle chiamate a `malloc()`;
* la prevenzione di condizioni di overflow;

Un’ulteriore differenza riguarda la leggibilità del codice, il quale risulta spesso meno commentato e presenta uno stile meno uniforme, soprattutto nella struttura dei blocchi di selezione. In presenza di istruzioni singole, la sintassi utilizzata varia frequentemente tra forme compatte su una sola riga e forme su più righe prive di parentesi graffe, riducendo la chiarezza e aumentando il rischio di errori in fase di manutenzione o modifica del codice.

Ad esempio, vengono utilizzate alternativamente le seguenti strutture:

```c
if (espressione) istruzione;
```

oppure:

```c
if (espressione)
    istruzione;
```

Al contrario, **Claude Sonnet v4.5** dimostra una maggiore precisione sia nello stile che nella strutturazione del codice, privilegiando soluzioni più leggibili, coerenti e robuste, con un uso più sistematico dei controlli di errore e una maggiore attenzione alla manutenibilità del software.

<a id="fine_report"></a>
## Considerazioni Finali

### Sfide nella correttezza algoritmica

1. **Verifica delle complessità:** 
    Il modello dichiara le complessità ma non le dimostra. È stato necessario 
    analizzare manualmente il codice per verificare che fossero rispettate.
    In particolare, la gestione delle collisioni tramite liste concatenate 
    garantisce O(k) dove k è la lunghezza della lista, compatibile con 
    "O(1) a meno di collisioni".

2. **Robustezza del codice:**
   Il modello genera codice difensivo (controlli NULL, gestione errori) 
   solo se esplicitamente richiesto. La richiesta iniziale di "effettuare 
   controllo di errori/parametri invalidi o nulli" è stata fondamentale.

3. **Necessità di iterazioni:**
   - Prima versione: funzionale ma con criticità (fprintf, mancanza controlli overflow)
   - Dopo feedback: codice production-ready
   - Numero iterazioni necessarie: 1-2 per componente

### Efficacia dell'approccio LLM-assisted

**Vantaggi:**
- Riduzione drastica del tempo di sviluppo
- Codice ben strutturato e commentato
- Implementazione corretta delle strutture dati base

**Limiti riscontrati:**
- Necessaria supervisione umana per correttezza algoritmica
- Problemi sottili (overflow, possibili omissioni su controllo errori) non identificati autonomamente

**Conclusione:**
Gli LLM si rivelano strumenti efficaci per generare codice, sia come punto di partenza sia per produrre rapidamente codice dettagliato che esegue operazioni relativamente semplici, contribuendo a ridurre i tempi di sviluppo. Tuttavia, resta fondamentale possedere competenze algoritmiche per verificarne la correttezza e valutare la complessità.
