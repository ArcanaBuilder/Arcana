# Changelog

All relevant changes to this project will be documented in this file.

## [0.6.0] - 2025-02-24
Major Release **Lushy Lion** (v 0.6.0)  

### Removed
- Attribute **flushcache**
- Removed task params

### Changes
- Cache module logic
- Improved job graph generation & visit
- Moved task instructions expansion logic

### Added
- Attribute **cache**, with commands: **track**, **untrack** and **store**

## [0.5.0] - 2025-02-11
### Changes
- Improved the Glob engine performance
- Minor performance improves

### Added
- Variable sum (+= semantic)
- Assert with recovery callbacks list
- **@glob** attribute, from now a glob pattern must be specified as glob
- **\_\_release__** builtin symbol
- Ability to search for multiple glob patterns and optionally map them into one

### Fixed Bugs
- Alingned variable declaration grammar regex to variable expansion regex 


## [0.4.3] - 2025-12-21
### Removed
- Module **Debug**

### Added
- CLI optons: **--value \<VALUE>**, **--pubs** and **--profiles** 

### Fixed Bugs
- **@main** attribute logic  


## [0.4.2] - 2025-12-20
### Changes
- Refactored Expader logic
- Added keyword **in** for **assert** statement  
- Added filesystem seach logic for **assert** statement via  
  **{fs:xxx}** expasion type  


## [0.4.1] - 2025-12-19

### Changes
- Improved windows support
- Reworked Makefile in order to include windows compilation 
- Messages error printing

### Added
- Variable expasion support for attribute @interpreter
- new **assert** statement:
```arcana
assert "{arc:__os__}" eq "linux" -> "reason message";
```

### Fixed Bugs
- Fixed script generation and execution


## [0.4.0] - 2025-12-18

### Added
- Custom Glob engine [BETA]
- Windows compatibility (minGW) [BETA]
- New attribute **@ifos**
- New allocator for builtin symbols

## [0.3.1] - 2025-12-13

### Added
- New statement **map** (same as @map attribute), with syntax:
```arcana
map <SOURCE> -> <TARGET>;
```

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
