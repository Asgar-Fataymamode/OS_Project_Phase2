# Contributing to MyShell Project

## 👥 Team Members

- **Mahmoud Kassem** (Person A) - Core functionality, I/O redirection, error handling
- **Asgar Fataymamode** (Person B) - Pipe implementation, compound commands

## 🚀 Development Workflow

### Getting Started
1. Clone the repository:
   ```bash
   git clone https://github.com/Mahmoud-K-Ismail/myshell-os-project.git
   cd myshell-os-project
   ```

2. Build and test:
   ```bash
   make clean
   make
   ./test_person_a_features.sh
   ```

### Branch Strategy
- `main` - Stable releases and major milestones
- `person-a-dev` - Mahmoud's development branch
- `person-b-dev` - Asgar's development branch  
- `integration` - Integration testing branch

### Development Process

#### For Person A (Mahmoud):
✅ **COMPLETED TASKS:**
- Core shell loop and prompt
- Command execution (fork/exec/wait)
- I/O redirection (input <, output >, error 2>)
- Combined redirections
- Error handling and memory management
- Documentation and testing

#### For Person B (Asgar):
🔄 **UPCOMING TASKS:**
- Pipe implementation (`|`)
- Multiple pipe support (`cmd1 | cmd2 | cmd3`)
- Compound commands with pipes and redirections
- Advanced error handling for pipes
- Integration testing with Person A's code

### Code Standards

#### C Code Style
```c
// Function header comment format
/**
 * Brief description of function
 * @param param1: Description of parameter
 * @return: Description of return value
 */
int function_name(int param1) {
    // Implementation with clear comments
    return 0;
}
```

#### Commit Message Format
```
<type>: <description>

✨ Features implemented:
- Feature 1
- Feature 2

🏗️ Technical details:
- Technical detail 1
- Technical detail 2
```

Types: `feat`, `fix`, `docs`, `test`, `refactor`, `chore`

### Testing Requirements

#### Before Committing:
1. **Compilation Test:**
   ```bash
   make clean && make
   ```

2. **Functionality Test:**
   ```bash
   ./test_person_a_features.sh  # Person A features
   # ./test_person_b_features.sh  # Person B features (when available)
   ```

3. **Memory Test:**
   ```bash
   make memcheck  # If valgrind is available
   ```

#### Integration Testing:
- Test all features work together
- Verify no regressions in existing functionality
- Check memory management across all features

### File Organization

```
myshell-os-project/
├── src/                     # Source files (if we reorganize)
│   ├── myshell.c
│   ├── shell_utils.c
│   └── shell_utils.h
├── tests/                   # Test files
│   ├── test_person_a_features.sh
│   └── test_person_b_features.sh
├── docs/                    # Documentation
│   ├── PERSON_A_*.md
│   └── PERSON_B_*.md
├── Makefile
├── README.md
└── CONTRIBUTING.md
```

### Integration Guidelines

#### For Person B (Asgar):
When implementing pipes, use the existing infrastructure:

```c
// Extend the existing command_t structure
typedef struct {
    char** argv;           // Already implemented
    int argc;              // Already implemented
    char* input_file;      // Already implemented
    char* output_file;     // Already implemented
    char* error_file;      // Already implemented
    // Add pipe-specific fields:
    int has_pipes;         // New: pipe flag
    int pipe_count;        // New: number of pipes
    // ... other pipe fields
} command_t;

// Extend existing functions:
command_t* parse_command_line(char* input);  // Extend for pipes
int execute_command(command_t* cmd);         // Extend for pipes
```

#### Integration Steps:
1. **Phase 1:** Implement basic single pipe (`cmd1 | cmd2`)
2. **Phase 2:** Extend to multiple pipes (`cmd1 | cmd2 | cmd3`)
3. **Phase 3:** Combine with existing redirections
4. **Phase 4:** Advanced compound commands
5. **Phase 5:** Final integration and testing

### Communication

#### Daily Standup (Async):
- What did you work on yesterday?
- What will you work on today?
- Any blockers or dependencies?

#### Code Review Process:
1. Create feature branch from main
2. Implement feature with tests
3. Create pull request
4. Other person reviews code
5. Address feedback
6. Merge to main

#### Issue Tracking:
Use GitHub Issues for:
- Bug reports
- Feature requests
- Integration questions
- Documentation updates

### Academic Integrity

#### Guidelines:
- All code must be original work by team members
- External resources must be properly cited
- No copying from other student projects
- Collaboration limited to assigned team members

#### Documentation:
- All sources and references must be documented
- Code comments must explain our logic, not just what the code does
- Report must detail our implementation approach

### Submission Checklist

#### Before Final Submission:
- [ ] All features implemented and tested
- [ ] Code compiles without warnings
- [ ] All tests pass
- [ ] Memory leaks checked and fixed
- [ ] Code properly commented
- [ ] Report completed
- [ ] README updated
- [ ] Final integration testing completed

#### Submission Package:
- Source code (.c, .h files)
- Makefile
- Report (PDF)
- Test results and screenshots
- All files in single .zip archive

### Emergency Contacts

- **Mahmoud Kassem:** [Contact Info]
- **Asgar Fataymamode:** [Contact Info]
- **Course TA:** [TA Contact]
- **Professor:** [Professor Contact]

---

**Remember:** This is an academic project with a hard deadline of October 2nd, 2025. Plan accordingly and communicate early about any issues!
