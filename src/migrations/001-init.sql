PRAGMA foreign_keys = ON;

CREATE TABLE IF NOT EXISTS schema_version (
    version INTEGER
);

INSERT INTO schema_version (version)
SELECT 1
WHERE NOT EXISTS (SELECT 1 FROM schema_version);

CREATE TABLE IF NOT EXISTS models (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT,
    description TEXT
);

CREATE TABLE IF NOT EXISTS terms (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    uuid TEXT NOT NULL,
    name TEXT,
    description TEXT,
    experiment_id INTEGER,
    numeric_value REAL,
    tr_value_l REAL,
    tr_value_m REAL,
    tr_value_h REAL,
    color_r INTEGER,
    color_g INTEGER,
    color_b INTEGER,
    type TEXT DEFAULT 'Node',
    FOREIGN KEY (experiment_id) REFERENCES experiments(id)
);

CREATE TABLE IF NOT EXISTS experiments (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    model_id INTEGER,
    timestamp TEXT,
    algorithm TEXT,
    use_fuzzy_values INTEGER,
    activation TEXT,
    predict_to_static INTEGER,
    metric TEXT,
    threshold REAL,
    steps_less_threshold INTEGER,
    fixed_steps INTEGER,
    FOREIGN KEY (model_id) REFERENCES models(id)
);

CREATE INDEX IF NOT EXISTS idx_experiments_model
ON experiments(model_id);

CREATE TABLE IF NOT EXISTS concepts (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    uuid TEXT NOT NULL,
    name TEXT,
    description TEXT,
    experiment_id INTEGER,
    term_id INTEGER,
    first_step INTEGER,
    x_pos REAL,
    y_pos REAL,
    FOREIGN KEY (experiment_id) REFERENCES experiments(id),
    FOREIGN KEY (term_id) REFERENCES terms(id)
);

CREATE INDEX IF NOT EXISTS idx_concepts_experiment
ON concepts(experiment_id);

CREATE TABLE IF NOT EXISTS weights (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    uuid TEXT NOT NULL,
    name TEXT,
    description TEXT,
    experiment_id INTEGER,
    term_id INTEGER,
    concept_from_id INTEGER,
    concept_to_id INTEGER,
    FOREIGN KEY (experiment_id) REFERENCES experiments(id),
    FOREIGN KEY (term_id) REFERENCES terms(id),
    FOREIGN KEY (concept_from_id) REFERENCES concepts(id),
    FOREIGN KEY (concept_to_id) REFERENCES concepts(id)
);

CREATE INDEX IF NOT EXISTS idx_weights_experiment
ON weights(experiment_id);

CREATE INDEX IF NOT EXISTS idx_weights_term
ON weights(term_id);

CREATE INDEX IF NOT EXISTS idx_weights_from
ON weights(concept_from_id);

CREATE INDEX IF NOT EXISTS idx_weights_to
ON weights(concept_to_id);

CREATE TABLE IF NOT EXISTS templates (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    uuid TEXT NOT NULL,
    name TEXT,
    description TEXT
);

CREATE TABLE IF NOT EXISTS templates_terms (
    template_id INTEGER,
    term_id INTEGER,
    PRIMARY KEY (template_id, term_id),
    FOREIGN KEY (template_id) REFERENCES templates(id),
    FOREIGN KEY (term_id) REFERENCES terms(id)
);

CREATE INDEX IF NOT EXISTS idx_templates_terms_template
ON templates_terms(template_id);

CREATE INDEX IF NOT EXISTS idx_templates_terms_term
ON templates_terms(term_id);

CREATE TABLE IF NOT EXISTS templates_concepts (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    uuid TEXT NOT NULL,
    template_id INTEGER,
    name TEXT,
    description TEXT,
    FOREIGN KEY (template_id) REFERENCES templates(id)
);

CREATE INDEX IF NOT EXISTS idx_templates_concepts_template
ON templates_concepts(template_id);

CREATE TABLE IF NOT EXISTS templates_weights (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    uuid TEXT NOT NULL,
    concept_from_id INTEGER,
    concept_to_id INTEGER,
    term_id INTEGER,
    name TEXT,
    description TEXT,
    FOREIGN KEY (concept_from_id) REFERENCES templates_concepts(id),
    FOREIGN KEY (concept_to_id) REFERENCES templates_concepts(id),
    FOREIGN KEY (term_id) REFERENCES terms(id)
);

CREATE INDEX IF NOT EXISTS idx_templates_weights_from
ON templates_weights(concept_from_id);

CREATE INDEX IF NOT EXISTS idx_templates_weights_to
ON templates_weights(concept_to_id);

CREATE INDEX IF NOT EXISTS idx_templates_weights_term
ON templates_weights(term_id);
