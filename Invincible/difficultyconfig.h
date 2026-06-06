#ifndef DIFFICULTYCONFIG_H
#define DIFFICULTYCONFIG_H

class DifficultyConfig {

public:

    // ── Física F1: Parábola ──────────────────────────────────
    float gravedadG;

    // ── Física F2: MAS de portales (Nivel 1) ─────────────────
    float amplitudMAS;
    float omegaMAS;

    // ── Física F3: Aceleración de Variantes (Nivel 2) ───────────
    float velVarianteBase;
    float aceleracionVariante;
    float velMaxVariante;

    // ── Física F5: Órbita de portales (Nivel 2) ──────────────
    float radioPortalNivel2;
    float omegaPortal;

    // ── Agente IA ────────────────────────────────────────────
    float agresividadIA;

    float velBaseLevy;          // velocidad de movimiento de Levy
    float cooldownEvasionLevy;  // segundos entre evasiones de Levy
    float velVoladora;          // velocidad de proyectiles VariantePortal
    float vidaJugador;          // vida inicial del jugador

    // ── Spawn de enemigos ────────────────────────────────────
    float frecuenciaSpawn;

    // ── Daño ─────────────────────────────────────────────────
    float danioPortal;
    float danioLevy;

    // ── Métodos de fábrica ───────────────────────────────────
    static DifficultyConfig facil();
    static DifficultyConfig normal();
    static DifficultyConfig dificil();

};

#endif
