#include "difficultyconfig.h"

DifficultyConfig DifficultyConfig::facil()
{
    DifficultyConfig cfg;

    // F1 — gravedad baja: parábolas más largas, más fáciles de esquivar
    cfg.gravedadG        = 200.0f;

    // F2 — MAS suave: portal oscila lento y con área pequeña
    cfg.amplitudMAS      = 30.0f;
    cfg.omegaMAS         = 1.0f;

    // F3 — Variante lentas
    cfg.velVarianteBase      = 60.0f;
    cfg.aceleracionVariante  = 20.0f;
    cfg.velMaxVariante       = 120.0f;

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

    cfg.velBaseLevy         = 80.0f;   // Levy lento
    cfg.cooldownEvasionLevy = 3.5f;    // evade poco
    cfg.velVoladora         = 130.0f;  // proyectiles lentos
    cfg.vidaJugador         = 500.0f;  // jugador aguanta más

    return cfg;
}

DifficultyConfig DifficultyConfig::normal()
{
    DifficultyConfig cfg;

    cfg.gravedadG        = 300.0f;

    cfg.amplitudMAS      = 50.0f;
    cfg.omegaMAS         = 1.5f;

    cfg.velVarianteBase      = 90.0f;
    cfg.aceleracionVariante  = 35.0f;
    cfg.velMaxVariante       = 200.0f;

    cfg.radioPortalNivel2 = 250.0f;
    cfg.omegaPortal       = 1.0f;

    cfg.agresividadIA    = 1.0f;

    cfg.frecuenciaSpawn  = 2.5f;

    cfg.danioPortal      = 6.0f;
    cfg.danioLevy        = 8.0f;

    cfg.velBaseLevy         = 120.0f;  // velocidad actual hardcoded
    cfg.cooldownEvasionLevy = 2.0f;    // actual
    cfg.velVoladora         = 200.0f;  // actual
    cfg.vidaJugador         = 400.0f;  // actual

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

    // F3 — Variantes rápidos
    cfg.velVarianteBase      = 130.0f;
    cfg.aceleracionVariante  = 60.0f;
    cfg.velMaxVariante      = 320.0f;

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

    cfg.velBaseLevy         = 185.0f;  // Levy muy rápido
    cfg.cooldownEvasionLevy = 0.7f;    // evade casi sin parar
    cfg.velVoladora         = 300.0f;  // proyectiles rápidos
    cfg.vidaJugador         = 260.0f;  // jugador fragil

    return cfg;
}
