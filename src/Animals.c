#include <stdio.h>	/* *printf */
#include <assert.h>	/* assert */
#include <string.h>	/* strncpy */
#include "Orcish.h"
#include "Animals.h"

enum Colour { PINK, RED, BLUE, YELLOW, BEIGE, COLOUR_END };
static const char *const colours[] = { "pink", "red", "blue","yellow", "beige"};
enum { BOTTOM, TOP, RIDING_END };

/* Abstract class Animal. */
struct AnimalVt;
struct Animal {
	const struct AnimalVt *vt;
	char name[16];
	enum Colour colour;
};
static const size_t animal_name_size = sizeof ((struct Animal *)0)->name
	/ sizeof *((struct Animal *)0)->name;
static void Animal_to_string(const struct Animal *const animal,
	char (*const a)[12]) {
	strncpy(*a, animal->name, sizeof *a / sizeof **a);
}
#define LIST_NAME Animal
#define LIST_TYPE struct Animal
#define LIST_TO_STRING &Animal_to_string
#include "List.h"

/* Class Connect. */
struct Connect {
	struct Animal *steed, *mount;
};
static void Animal_mount_migrate(struct Animal *const animal, const struct Migrate *const migrate); /* prototype */
static void connect_migrate(struct Connect *const this,
	const struct Migrate *const migrate) {
	assert(this && migrate && this->steed && this->mount);
	Animal_mount_migrate(this->steed, migrate);
	Animal_mount_migrate(this->mount, migrate);
}
#define POOL_NAME Connect
#define POOL_TYPE struct Connect
#define POOL_MIGRATE_EACH &connect_migrate
#include "Pool.h"

/* Class Sloth extends Animal. */
struct Sloth {
	struct AnimalListNode animal;
	unsigned hours_slept;
};
static void sloth_migrate(struct Sloth *const this,
	const struct Migrate *const migrate) {
	assert(this && migrate);
	AnimalListNodeMigrate(&this->animal, migrate);
}
#define POOL_NAME Sloth
#define POOL_TYPE struct Sloth
#define POOL_MIGRATE_EACH &sloth_migrate
#include "Pool.h"

/* Class Emu extends Animal. */
struct Emu {
	struct AnimalListNode animal;
	char favourite_letter;
};
static void emu_migrate(struct Emu *const this,
	const struct Migrate *const migrate) {
	assert(this && migrate);
	AnimalListNodeMigrate(&this->animal, migrate);
}
#define POOL_NAME Emu
#define POOL_TYPE struct Emu
#define POOL_MIGRATE_EACH &emu_migrate
#include "Pool.h"

/* Class BadEmu extends Emu. */
struct BadEmu {
	struct Emu emu;
	struct Connect *mount;
	char muhaha[12];
};
static void bad_emu_migrate(struct BadEmu *const this,
	const struct Migrate *const migrate) {
	assert(this && migrate);
	AnimalListNodeMigrate(&this->emu.animal, migrate);
}
#define POOL_NAME BadEmu
#define POOL_TYPE struct BadEmu
#define POOL_MIGRATE_EACH &bad_emu_migrate
#include "Pool.h"

/* Class Llama extends Animal. */
struct Llama {
	struct AnimalListNode animal;
	struct Connect *steed;
	unsigned chomps;
};
static void llama_migrate(struct Llama *const this,
	const struct Migrate *const migrate) {
	assert(this && migrate);
	AnimalListNodeMigrate(&this->animal, migrate);
}
#define POOL_NAME Llama
#define POOL_TYPE struct Llama
#define POOL_MIGRATE_EACH &llama_migrate
#include "Pool.h"

/* Class Lemur extends Animal. */
struct Lemur {
	struct AnimalListNode animal;
	struct Connect *mount;
};
static void lemur_migrate(struct Lemur *const this,
	const struct Migrate *const migrate) {
	assert(this && migrate);
	AnimalListNodeMigrate(&this->animal, migrate);
}
#define POOL_NAME Lemur
#define POOL_TYPE struct Lemur
#define POOL_MIGRATE_EACH &lemur_migrate
#include "Pool.h"

/* Class Bear extends Animal. We have always two or less, so we don't need to
 define a {Pool}. */
struct Bear {
	struct AnimalListNode animal;
	int is_active;
	struct Connect *mount;
};

/* Animal list with backing. These are the storage structures. */
struct Animals {
	struct AnimalList list;
	struct ConnectPool *connects;
	struct SlothPool *sloths;
	struct EmuPool *emus;
	struct BadEmuPool *bad_emus;
	struct LlamaPool *llamas;
	struct LemurPool *lemurs;
	struct Bear bears[2];
};
static const unsigned no_bears = sizeof(((struct Animals *)0)->bears) / sizeof(*((struct Animals *)0)->bears);

typedef void (*AnimalAction)(struct Animal *const);
typedef void (*AnimalsAction)(struct Animals *const, struct Animal *const);
typedef struct Connect *const*(*AnimalConnectField)(const struct Animal *const);
typedef void (*AnimalConnectMigrate)(struct Animal *const, const struct Migrate *const);

/*********/

struct AnimalVt {
	const char kind[16];
	AnimalsAction delete;
	AnimalAction act/*transmogrify*/;
	AnimalConnectMigrate mount_migrate;
	AnimalConnectField steed, mount;
};

/** @implements <Animal, [Animals]>BiAction */
static void Animal_delete(struct Animal *const animal,
	void *const void_animals) {
	struct Animals *const animals = void_animals;
	if(!animals || !animal) return;
	animal->vt->delete(animals, animal);
}
static void Sloth_delete(struct Animals *const animals,
	struct Sloth *const sloth) {
	printf("Bye %s.\n", sloth->animal.data.name);
	AnimalListRemove(&sloth->animal.data);
	SlothPoolRemove(animals->sloths, sloth);
}
static void Emu_delete(struct Animals *const animals,
	struct Emu *const emu) {
	printf("Bye %s.\n", emu->animal.data.name);
	AnimalListRemove(&emu->animal.data);
	EmuPoolRemove(animals->emus, emu);
}
static void BadEmu_delete(struct Animals *const animals,
	struct BadEmu *const bad_emu) {
	printf("%s dissapers in a puff of smoke.\n", bad_emu->emu.animal.data.name);
	AnimalListRemove(&bad_emu->emu.animal.data);
	BadEmuPoolRemove(animals->bad_emus, bad_emu);
	/* @fixme Mount. */
}
static void Lemur_delete(struct Animals *const animals,
	struct Lemur *const lemur) {
	printf("Bye %s.\n", lemur->animal.data.name);
	AnimalListRemove(&lemur->animal.data);
	LemurPoolRemove(animals->lemurs, lemur);
	/* @fixme Mount. */
}
static void Llama_delete(struct Animals *const animals,
	struct Llama *const llama) {
	printf("Bye %s.\n", llama->animal.data.name);
	AnimalListRemove(&llama->animal.data);
	LlamaPoolRemove(animals->llamas, llama);
	/* @fixme Mount. */
}
static void Bear_delete(struct Animals *const animals,
	struct Bear *const bear) {
	if(!bear->is_active) return;
	printf("Bye %s.\n", bear->animal.data.name);
	AnimalListRemove(&bear->animal.data);
	bear->is_active = 0;
	(void)animals;
	/* @fixme Mount. */
}

/** @implements <Animal>Action */
static void Animal_act(struct Animal *const animal) {
	assert(animal);
	animal->vt->act(animal);
}
static void Sloth_act(struct Sloth *const sloth) {
	printf("%s %s has favourite colour %s and has been sleeping %u hours.\n",
		sloth->animal.data.vt->kind, sloth->animal.data.name,
		colours[sloth->animal.data.colour], sloth->hours_slept);
}
static void Emu_act(struct Emu *const emu) {
	printf("%s %s has favourite colour %s and favourite letter %c.\n",
		emu->animal.data.vt->kind, emu->animal.data.name,
		colours[emu->animal.data.colour], emu->favourite_letter);
}
static void BadEmu_act(struct BadEmu *const bad_emu) {
	char riding[64] = "";
	if(bad_emu->mount) {
		struct Connect *const connect = bad_emu->mount;
		assert(connect->steed && connect->mount == &bad_emu->emu.animal.data);
		sprintf(riding, " They are riding on %s the %s.",
			connect->steed->name, connect->steed->vt->kind);
	}
	printf("%s %s has favourite colour %s and favourite letter %c; "
		"he is mumbling \"%s.\"%s\n", bad_emu->emu.animal.data.vt->kind,
		bad_emu->emu.animal.data.name, colours[bad_emu->emu.animal.data.colour],
		bad_emu->emu.favourite_letter, bad_emu->muhaha, riding);
}
static void Lemur_act(struct Lemur *const lemur) {
	printf("%s %s.\n", lemur->animal.data.vt->kind, lemur->animal.data.name);
}
static void Llama_act(struct Llama *const llama) {
	printf("%s %s has chomped %u fingers today.\n", llama->animal.data.vt->kind,
		llama->animal.data.name, llama->chomps);
}
static void Bear_act(struct Bear *const bear) {
	char riding[64] = "chilling";
	if(bear->mount) {
		struct Connect *const connect = bear->mount;
		assert(connect->steed && connect->mount == &bear->animal.data);
		sprintf(riding, "riding on %s the %s",
			connect->steed->name, connect->steed->vt->kind);
	}
	printf("%s %s is %s.\n", bear->animal.data.vt->kind,
		bear->animal.data.name, riding);
}

/** @implements AnimalConnectMigrate */
static void Animal_mount_migrate(struct Animal *const animal, const struct Migrate *const migrate) {
	assert(animal && animal->vt->mount_migrate);
	animal->vt->mount_migrate(animal, migrate);
}
static void BadEmu_mount_migrate(struct BadEmu *const bad_emu, const struct Migrate *const migrate) {
	ConnectPoolMigratePointer(&bad_emu->mount, migrate);
}
static void Lemur_mount_migrate(struct Lemur *const lemur, const struct Migrate *const migrate) {
	ConnectPoolMigratePointer(&lemur->mount, migrate);
}
static void Llama_mount_migrate(struct Llama *const llama, const struct Migrate *const migrate) {
	ConnectPoolMigratePointer(&llama->steed, migrate);
}
static void Bear_mount_migrate(struct Bear *const bear, const struct Migrate *const migrate) {
	ConnectPoolMigratePointer(&bear->mount, migrate);
}

/** @implements AnimalConnectField */
static struct Connect *const*Animal_steed(const struct Animal *const animal) {
	AnimalConnectField steed;
	assert(animal);
	if(!(steed = animal->vt->steed)) return 0;
	return steed(animal);
}
static struct Connect *const*Llama_steed(const struct Llama *const llama) {
	return &llama->steed;
}

/** @implements AnimalConnectField */
static struct Connect *const*Animal_mount(const struct Animal *const animal) {
	AnimalConnectField mount;
	assert(animal);
	if(!(mount = animal->vt->mount)) return 0;
	return mount(animal);
}
static struct Connect *const*BadEmu_mount(const struct BadEmu *const bad_emu) {
	return &bad_emu->mount;
}
static struct Connect *const*Lemur_mount(const struct BadEmu *const lemur) {
	return &lemur->mount;
}
static struct Connect *const*Bear_mount(const struct Bear *const bear) {
	return &bear->mount;
}

/* Static data containing the functions defined above. */
static struct AnimalVt Sloth_vt = {
	"Sloth",
	(AnimalsAction)&Sloth_delete,
	(AnimalAction)&Sloth_act,
	0,
	0,
	0
};
static struct AnimalVt Emu_vt = {
	"Emu",
	(AnimalsAction)&Emu_delete,
	(AnimalAction)&Emu_act,
	0,
	0,
	0
};
static struct AnimalVt BadEmu_vt = {
	"Emu",
	(AnimalsAction)&BadEmu_delete,
	(AnimalAction)&BadEmu_act,
	(AnimalConnectMigrate)&BadEmu_mount_migrate,
	0,
	(AnimalConnectField)&BadEmu_mount,
};
static struct AnimalVt Lemur_vt = {
	"Lemur",
	(AnimalsAction)&Lemur_delete,
	(AnimalAction)&Lemur_act,
	(AnimalConnectMigrate)&Lemur_mount_migrate,
	0,
	(AnimalConnectField)&Lemur_mount
};
static struct AnimalVt Llama_vt = {
	"Llama",
	(AnimalsAction)&Llama_delete,
	(AnimalAction)&Llama_act,
	(AnimalConnectMigrate)&Llama_mount_migrate,
	(AnimalConnectField)&Llama_steed,
	0
};
static struct AnimalVt Bear_vt = {
	"Bear",
	(AnimalsAction)&Bear_delete,
	(AnimalAction)&Bear_act,
	(AnimalConnectMigrate)Bear_mount_migrate,
	0,
	(AnimalConnectField)&Bear_mount,
};

/************/

/** Only called from constructors of children. */
static void Animal_filler(struct Animal *const animal,
	const struct AnimalVt *const vt) {
	assert(animal && vt);
	animal->vt     = vt;
	animal->colour = (enum Colour)(1.0 * COLOUR_END * rand() / (RAND_MAX +1.0));
	Orcish(animal->name, sizeof animal->name);
}
/** Destructor. */
void Animals_(struct Animals **const animalsp) {
	struct Animals *animals;
	if(!animalsp || !(animals = *animalsp)) return;
	AnimalListClear(&animals->list);
	EmuPool_(&animals->emus);
	SlothPool_(&animals->sloths);
}
/** Constructor. */
struct Animals *Animals(void) {
	struct Animals *a;
	struct Bear *bear, *end;
	int is_success = 0;
	const char *e = "null";
	if(!(a = malloc(sizeof *a))) { perror("Animals"); Animals_(&a); return 0; }
	AnimalListClear(&a->list);
	a->connects = 0;
	a->sloths   = 0;
	a->emus     = 0;
	a->bad_emus = 0;
	a->llamas   = 0;
	a->lemurs   = 0;
	for(bear = a->bears, end = bear + no_bears; bear < end; bear++)
		bear->is_active = 0;
	errno = 0; do {
		if(!(e = "connects", a->connects = ConnectPool())
			|| !(e = "sloths", a->sloths = SlothPool())
			|| !(e = "emus", a->emus = EmuPool())
			|| !(e = "bad_emus", a->bad_emus = BadEmuPool())
			|| !(e = "llamas", a->llamas = LlamaPool())
			|| !(e = "lemurs", a->lemurs = LemurPool())
		) break;
		is_success = 1;
	} while(0); if(!is_success) {
		fprintf(stderr, "Animals, constructing %s: %s.\n",
			e, strerror(errno));
		Animals_(&a);
	}
	return a;
}
struct Sloth *Sloth(struct Animals *const animals) {
	struct Sloth *sloth;
	if(!animals) return 0;
	if(!(sloth = SlothPoolNew(animals->sloths))) return 0;
	Animal_filler(&sloth->animal.data, &Sloth_vt);
	sloth->hours_slept = (int)(10.0 * rand() / RAND_MAX) + 4;
	AnimalListPush(&animals->list, &sloth->animal.data);
	return sloth;
}
struct Emu *Emu(struct Animals *const animals) {
	struct Emu *emu;
	if(!animals) return 0;
	if(!(emu = EmuPoolNew(animals->emus))) return 0;
	Animal_filler(&emu->animal.data, &Emu_vt);
	emu->favourite_letter = 'a' + (char)(26.0 * rand() / RAND_MAX);
	AnimalListPush(&animals->list, &emu->animal.data);
	return emu;
}
struct BadEmu *BadEmu(struct Animals *const animals) {
	struct BadEmu *emu;
	if(!animals) return 0;
	if(!(emu = BadEmuPoolNew(animals->bad_emus))) return 0;
	Animal_filler(&emu->emu.animal.data, &BadEmu_vt);
	emu->emu.favourite_letter = 'a' + (char)(26.0 * rand() / RAND_MAX);
	emu->mount = 0;
	Orcish(emu->muhaha, sizeof emu->muhaha);
	AnimalListPush(&animals->list, &emu->emu.animal.data);
	return emu;
}
struct Llama *Llama(struct Animals *const animals) {
	struct Llama *llama;
	if(!animals) return 0;
	if(!(llama = LlamaPoolNew(animals->llamas))) return 0;
	Animal_filler(&llama->animal.data, &Llama_vt);
	llama->steed  = 0;
	llama->chomps = 5 + 10 * rand() / RAND_MAX;
	AnimalListPush(&animals->list, &llama->animal.data);
	return llama;
}
struct Lemur *Lemur(struct Animals *const animals) {
	struct Lemur *lemur;
	if(!animals) return 0;
	if(!(lemur = LemurPoolNew(animals->lemurs))) return 0;
	Animal_filler(&lemur->animal.data, &Lemur_vt);
	AnimalListPush(&animals->list, &lemur->animal.data);
	return lemur;
}
void Bear(struct Animals *const animals, const unsigned no, const char *const name) {
	struct Bear *bear;
	if(!animals || no >= no_bears) return;
	bear = animals->bears + no;
	if(bear->is_active) { fprintf(stderr, "Bear is active.\n"); return; }
	Animal_filler(&bear->animal.data, &Bear_vt);
	if(name) strncpy(bear->animal.data.name, name, animal_name_size - 1),
		bear->animal.data.name[animal_name_size - 1] = '\0';
	bear->is_active = 1;
	bear->mount = 0;
	AnimalListPush(&animals->list, &bear->animal.data);
}

/** @implements <Animal, [size_t *]>BiAction */
static void Animal_count(struct Animal *const animal, void *const pcount) {
	assert(animal && pcount);
	(*(size_t *)pcount)++;
}
void AnimalsAct(struct Animals *const animals) {
	size_t count = 0;
	if(!animals) return;
	AnimalListBiForEach(&animals->list, &Animal_count, &count);
	printf("There are %lu animals.\n", (long unsigned)count);
	AnimalListForEach(&animals->list, &Animal_act);
}
void AnimalsClear(struct Animals *const animals) {
	if(!animals) return;
	AnimalListBiForEach(&animals->list, &Animal_delete, animals);
}
