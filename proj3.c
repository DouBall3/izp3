/*
 * This code was created by Ondrej Dohnal (xdohna45)
 */

/**
 * Kostra programu pro 3. projekt IZP 2018/19
 *
 * Jednoducha shlukova analyza: 2D nejblizsi soused.
 * Single linkage
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h> // sqrtf
#include <limits.h> // INT_MAX

/*****************************************************************
 * Ladici makra. Vypnout jejich efekt lze definici makra
 * NDEBUG, napr.:
 *   a) pri prekladu argumentem prekladaci -DNDEBUG
 *   b) v souboru (na radek pred #include <assert.h>
 *      #define NDEBUG
 */
#ifdef NDEBUG
#define debug(s)
#define dfmt(s, ...)
#define dint(i)
#define dfloat(f)
#else

// vypise ladici retezec
#define debug(s) printf("- %s\n", s)

// vypise formatovany ladici vystup - pouziti podobne jako printf
#define dfmt(s, ...) printf(" - "__FILE__":%u: "s"\n",__LINE__,__VA_ARGS__)

// vypise ladici informaci o promenne - pouziti dint(identifikator_promenne)
#define dint(i) printf(" - " __FILE__ ":%u: " #i " = %d\n", __LINE__, i)

// vypise ladici informaci o promenne typu float - pouziti
// dfloat(identifikator_promenne)
#define dfloat(f) printf(" - " __FILE__ ":%u: " #f " = %g\n", __LINE__, f)

#endif

/*****************************************************************
 * Deklarace potrebnych datovych typu:
 *
 * TYTO DEKLARACE NEMENTE
 *
 *   struct obj_t - struktura objektu: identifikator a souradnice
 *   struct cluster_t - shluk objektu:
 *      pocet objektu ve shluku,
 *      kapacita shluku (pocet objektu, pro ktere je rezervovano
 *          misto v poli),
 *      ukazatel na pole shluku.
 */

struct obj_t {
    int id;
    float x;
    float y;
};

struct cluster_t {
    int size;
    int capacity;
    struct obj_t *obj;
};

/*****************************************************************
 * Deklarace potrebnych funkci.
 *
 * PROTOTYPY FUNKCI NEMENTE
 *
 * IMPLEMENTUJTE POUZE FUNKCE NA MISTECH OZNACENYCH 'TODO'
 *
 */

void usage()
{
    fputs("Usage: ./proj3 <file> [N]\n",stdout);
}


/*
 Inicializace shluku 'c'. Alokuje pamet pro cap objektu (kapacitu).
 Ukazatel NULL u pole objektu znamena kapacitu 0.
*/
void init_cluster(struct cluster_t *c, int cap)
{
    assert(c != NULL);
    assert(cap >= 0);

    c->size = 0;
    if(cap > 0)
    {
        c->obj = malloc(cap * sizeof(struct obj_t) + 1);
        if(c->obj)
        {
            c->capacity = cap;
            return;
        }
    }
    c->capacity = 0;
    c->obj = NULL;

}

/*
 Odstraneni vsech objektu shluku a inicializace na prazdny shluk.
 */
void clear_cluster(struct cluster_t *c)
{
    c->obj = NULL;
    free(c->obj);
    init_cluster(c, 0);

}

/// Chunk of cluster objects. Value recommended for reallocation.
const int CLUSTER_CHUNK = 10;

/*
 Zmena kapacity shluku 'c' na kapacitu 'new_cap'.
 */
struct cluster_t *resize_cluster(struct cluster_t *c, int new_cap)
{
    // TUTO FUNKCI NEMENTE
    assert(c);
    assert(c->capacity >= 0);
    assert(new_cap >= 0);

    if (c->capacity >= new_cap)
        return c;

    size_t size = sizeof(struct obj_t) * new_cap;

    void *arr = realloc(c->obj, size);
    if (arr == NULL)
        return NULL;

    c->obj = arr;
    c->capacity = new_cap;
    return c;
}

/*
 Prida objekt 'obj' na konec shluku 'c'. Rozsiri shluk, pokud se do nej objekt
 nevejde.
 */
void append_cluster(struct cluster_t *c, struct obj_t obj)
{
    if(c->capacity <= c->size)
    {
        resize_cluster(c, c->capacity+CLUSTER_CHUNK);
    }
    c->obj[c->size++] = obj;
}

/*
 Seradi objekty ve shluku 'c' vzestupne podle jejich identifikacniho cisla.
 */
void sort_cluster(struct cluster_t *c);

/*
 Do shluku 'c1' prida objekty 'c2'. Shluk 'c1' bude v pripade nutnosti rozsiren.
 Objekty ve shluku 'c1' budou serazeny vzestupne podle identifikacniho cisla.
 Shluk 'c2' bude nezmenen.
 */
void merge_clusters(struct cluster_t *c1, struct cluster_t *c2)
{
    assert(c1 != NULL);
    assert(c2 != NULL);

    for(int i = 0; i < c2->size; i++)
    {
        append_cluster(c1,c2->obj[i]);
    }
    sort_cluster(c1);
}

/**********************************************************************/
/* Prace s polem shluku */

/*
 Odstrani shluk z pole shluku 'carr'. Pole shluku obsahuje 'narr' polozek
 (shluku). Shluk pro odstraneni se nachazi na indexu 'idx'. Funkce vraci novy
 pocet shluku v poli.
*/
int remove_cluster(struct cluster_t *carr, int narr, int idx)
{
    assert(idx < narr);
    assert(narr > 0);
    if(!(idx < narr)) return -1;
    if(!(narr > 0)) return -1;
    carr[idx].obj = NULL;
    carr->size--;
    return narr-1;
}

/*
 Pocita Euklidovskou vzdalenost mezi dvema objekty.
 */
float obj_distance(struct obj_t *o1, struct obj_t *o2)
{
    assert(o1 != NULL);
    assert(o2 != NULL);
    if(o1 == NULL) return -1;
    if(o2 == NULL) return -1;
    float distance = sqrt(pow(o1->x - o2->x, 2) + pow(o1->y - o2->y, 2));
    return distance;
}

/*
 Pocita vzdalenost dvou shluku.
*/
float cluster_distance(struct cluster_t *c1, struct cluster_t *c2)
{
    assert(c1 != NULL);
    assert(c1->size > 0);
    assert(c2 != NULL);
    assert(c2->size > 0);

    if(c1 == NULL) return -1;
    if(c2 == NULL) return -1;
    if(c1->size <= 0) return -1;
    if(c2->size <= 0) return -1;

    float distance = 0;
    for(int j = 0; j < c1->size; j++)
    {
        for( int i = 0; i < c2->size;i++)
        {
            if(distance > obj_distance(&c1->obj[i],&c2->obj[j]))
            {
                distance = obj_distance(&c1->obj[i],&c2->obj[j]);
            }
        }
    }
    return distance;
}

/*
 Funkce najde dva nejblizsi shluky. V poli shluku 'carr' o velikosti 'narr'
 hleda dva nejblizsi shluky. Nalezene shluky identifikuje jejich indexy v poli
 'carr'. Funkce nalezene shluky (indexy do pole 'carr') uklada do pameti na
 adresu 'c1' resp. 'c2'.
*/
void find_neighbours(struct cluster_t *carr, int narr, int *c1, int *c2)
{
    assert(narr > 0);

    float distance, min_distance = -1;
    for(int i = 0; i < narr; i++)
    {
        for(int j = i+1; j < narr; j++)
        {
	    distance = cluster_distance(&carr[i],&carr[j]);
	    if(min_distance == -1 || distance < min_distance)
	    {
		min_distance = distance;
		*c1 = i;
		*c2 = j;
	    }
        }
    }
}

// pomocna funkce pro razeni shluku
static int obj_sort_compar(const void *a, const void *b)
{
    // TUTO FUNKCI NEMENTE
    const struct obj_t *o1 = (const struct obj_t *)a;
    const struct obj_t *o2 = (const struct obj_t *)b;
    if (o1->id < o2->id) return -1;
    if (o1->id > o2->id) return 1;
    return 0;
}

/*
 Razeni objektu ve shluku vzestupne podle jejich identifikatoru.
*/
void sort_cluster(struct cluster_t *c)
{
    // TUTO FUNKCI NEMENTE
    qsort(c->obj, c->size, sizeof(struct obj_t), &obj_sort_compar);
}

/*
 Tisk shluku 'c' na stdout.
*/
void print_cluster(struct cluster_t *c)
{
    // TUTO FUNKCI NEMENTE
    for (int i = 0; i < c->size; i++)
    {
        if (i) putchar(' ');
        printf("%d[%g,%g]", c->obj[i].id, c->obj[i].x, c->obj[i].y);
    }
    putchar('\n');
}

/*
 Ze souboru 'filename' nacte objekty. Pro kazdy objekt vytvori shluk a ulozi
 jej do pole shluku. Alokuje prostor pro pole vsech shluku a ukazatel na prvni
 polozku pole (ukalazatel na prvni shluk v alokovanem poli) ulozi do pameti,
 kam se odkazuje parametr 'arr'. Funkce vraci pocet nactenych objektu (shluku).
 V pripade nejake chyby uklada do pameti, kam se odkazuje 'arr', hodnotu NULL.
*/
int load_clusters(char *filename, struct cluster_t **arr)
{
    assert(arr != NULL);

    FILE *file;
    file = fopen(filename, "r");
    if(!file)
    {
        fputs("Error, couldn't open file!",stderr);
        return -1;
    }
    char *line = malloc(sizeof(line)* sizeof(char)+1);
    int count, id;
    float x, y;
    fscanf(file,"count=%d",&count);
    *arr = malloc(sizeof(struct cluster_t) * count);
    for(int i = 0; i < count; i++)
    {
        init_cluster(&(*arr)[i],count);
        fscanf(file, "%d %f %f", &id, &x, &y);
        struct obj_t obj;
        obj.id = id;
        obj.x = x;
        obj.y = y;
        append_cluster(&(*arr)[i],obj);
    }
    free(line);
    fclose(file);
    return count;
}

/*
 Tisk pole shluku. Parametr 'carr' je ukazatel na prvni polozku (shluk).
 Tiskne se prvnich 'narr' shluku.
*/
void print_clusters(struct cluster_t *carr, int narr)
{
    printf("Clusters:\n");
    for (int i = 0; i < narr; i++)
    {
        printf("cluster %d: ", i);
        print_cluster(&carr[i]);
    }
}

int main(int argc, char *argv[])
{
    struct cluster_t *clusters;
    int c1_idx = 0, c2_idx = 0, previous_c1_size = 0;

    if(argc != 3 && argc != 2)
    {
        usage();
        return 1;
    }
    char* end;
    int n = (int) strtol(argv[2],&end,10);
    if(!n)
    {
        fputs("N has to be number",stderr);
        return 1;
    }
    int lines = load_clusters(argv[1],&clusters);
    printf("Nacteno lines: %d\n",lines);
    if(n > lines)
    {
        fputs("N can't be larger than number of objects",stderr);
        return 1;
    }
    while (lines < n) {
        // hledani sousednich shluku
        find_neighbours(clusters, lines, &c1_idx, &c2_idx);
	printf("c1: %d c2: %d\n",c1_idx,c2_idx);
        // spojovani sousednich shluku do shluku na indexu `c1_idx`
        previous_c1_size = clusters[c1_idx].size;
        merge_clusters(&clusters[c1_idx], &clusters[c2_idx]);
        if (clusters[c2_idx].size > 0 && clusters[c1_idx].size != previous_c1_size + clusters[c2_idx].size) {
            fputs("Chyba alokace pameti.",stderr);
            return -1;
        }
        // odstraneni shluku v poli z indexu `c2_idx`
        lines = remove_cluster(clusters, lines, c2_idx);
        printf("V while: %d\n",lines);
    }
    printf("Lines: %d\n",lines);
    print_clusters(clusters,lines);
    for(int i = 0; i < lines; i++)
    {
	free(clusters[i].obj);
    }
    free(clusters);
    return 0;
}


