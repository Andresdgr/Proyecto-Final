#include "difficultyconfig.h"

DifficultyConfig DifficultyConfig::facil()
{
    DifficultyConfig cfg;

    // F1 — gravedad baja: parábolas más largas, más fáciles de esquivar
    cfg.gravedadG        = 200.0f;

    // F2 — MAS suave: portal oscila lento y con área pequeña
    cfg.amplitudMAS      = 30.0f;
    cfg.omegaMAS         = 1.0f;

    // F3 — Clones lentos
    cfg.velClonBase      = 60.0f;
    cfg.aceleracionClon  = 20.0f;
    cfg.velMaxClon       = 120.0f;

    // F5 — Portales orbitales lejos y lentos
    cfg.radioPortalNivel2 = 350.0f;
    cfg.omegaPortal       = 0.5f;

    // IA — Levy poco agresivo
    cfg.agresividadIA    = 0.7f;

    // Spawn — aparecen pocos clones
    cfg.frecuenciaSpawn  = 4.0f;

    // Daño
    cfg.danioPortal      = 3.0f;
    cfg.danioLevy        = 5.0f;

    return cfg;
}

DifficultyConfig DifficultyConfig::normal()
{
    DifficultyConfig cfg;

    cfg.gravedadG        = 300.0f;

    cfg.amplitudMAS      = 50.0f;
    cfg.omegaMAS         = 1.5f;

    cfg.velClonBase      = 90.0f;
    cfg.aceleracionClon  = 35.0f;
    cfg.velMaxClon       = 200.0f;

    cfg.radioPortalNivel2 = 250.0f;
    cfg.omegaPortal       = 1.0f;

    cfg.agresividadIA    = 1.0f;

    cfg.frecuenciaSpawn  = 2.5f;

    cfg.danioPortal      = 6.0f;
    cfg.danioLevy        = 8.0f;

    return cfg;
}

DifficultyConfig DifficultyConfig::dificil()
{
    DifficultyConfig cfg;

    // F1 — gravedad alta: parábolas cortas y rápidas
    cfg.gravedadG        = 450.0f;

    // F2 — MAS agresivo: portal oscila rápido con área grande
    cfg.amplitudMAS      = 80.0f;
    cfg.omegaMAS         = 2.5f;

    // F3 — Clones rápidos
    cfg.velClonBase      = 130.0f;
    cfg.aceleracionClon  = 60.0f;
    cfg.velMaxClon       = 320.0f;

    // F5 — Portales orbitales cerca y rápidos
    cfg.radioPortalNivel2 = 160.0f;
    cfg.omegaPortal       = 1.8f;

    // IA — Levy muy agresivo
    cfg.agresividadIA    = 1.5f;

    // Spawn — clones frecuentes
    cfg.frecuenciaSpawn  = 1.5f;

    // Daño alto
    cfg.danioPortal      = 12.0f;
    cfg.danioLevy        = 12.0f;

    return cfg;
}
