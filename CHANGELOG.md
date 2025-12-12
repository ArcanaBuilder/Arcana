# Changelog

All relevant changes to this project will be documented in this file.

## [0.3.0] - 2025-12-12

### Added
- New builtin symbols:
  - \_\_main__
  - \_\_root__
  - \_\_threads__
  - \_\_max_threads__
  - \_\_os__
  - \_\_arch__ 
- Updated script generation logic in cache module.
- CRLF normalization in Lexer module.
- New CLI option: **--silent**

### Fixed Bugs
- Fixed 'inline' logic expasion of glob variables.
- Fixed profile switching not invalidating cache.

---

## [0.2.0] - 2025-12-07

### Added
- New cache invalidation logic for build graph.
- New attribute **exclude**

### Fixed Bugs
- Fixed minor bugs.

---

## [0.1.0] - 2025-12-05

### Added
- First public version of Arcana.
- Parser, Semantic analysis and Job generation.
- Job executor.
- Basic cache logic.
