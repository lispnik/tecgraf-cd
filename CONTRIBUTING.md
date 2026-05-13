# Contributing to CD

We welcome contributions to the CD graphics library! Here's how to get started.

## Development Setup

1. **Fork and clone the repository**:
   ```bash
   git clone https://github.com/yourusername/tecgraf-cd.git
   cd tecgraf-cd
   ```

2. **Install dependencies** (see [README.md](README.md) for platform-specific instructions)

3. **Set up the IM dependency**:
   ```bash
   git clone https://github.com/lispnik/tecgraf-im.git im-git
   cd im-git && mkdir build && cd build
   cmake .. -DCMAKE_INSTALL_PREFIX=../install
   make && make install
   cd ../..
   ```

4. **Build CD**:
   ```bash
   mkdir build && cd build
   cmake .. -DIM_INCLUDE_DIR=../im-git/install/include -DIM_LIBRARY=../im-git/install/lib/libim.so
   make
   ```

## Making Changes

1. **Create a feature branch**:
   ```bash
   git checkout -b feature/your-feature-name
   ```

2. **Make your changes** following the coding standards below

3. **Add tests** if applicable (in `test/` directory)

4. **Test your changes**:
   ```bash
   cd build
   make test
   ```

5. **Commit with descriptive messages**:
   ```bash
   git commit -m "Add support for new drawing primitive XYZ"
   ```

## Coding Standards

### C Code Style
- Use 2-space indentation
- Follow existing naming conventions:
  - Functions: `cdCanvasSomething()`, `cdContextSomething()`
  - Constants: `CD_SOMETHING`
  - Types: `cdSomething`
- Keep lines under 100 characters
- Use descriptive variable names

### Example:
```c
void cdCanvasNewFunction(cdCanvas* canvas, int param1, double param2) {
  if (!canvas) 
    return;
    
  canvas->some_field = param1;
  canvas->other_field = (int)(param2 * 100);
  
  // Call backend-specific implementation
  canvas->cxNewFunction(canvas->ctxcanvas, param1, param2);
}
```

### CMake Style
- Use lowercase function names
- Indent with 2 spaces
- Use `target_*` commands instead of global commands

## Types of Contributions

### Bug Fixes
- Check existing issues first
- Include test case that reproduces the bug
- Test on multiple platforms if possible

### New Features
- Discuss in an issue first for larger changes
- Consider backend compatibility
- Add documentation and examples
- Update CMake build system if needed

### Documentation
- Keep README.md up to date
- Add API documentation in headers
- Include examples in `test/` or `html/examples/`

### Platform Support
- Test on the target platform
- Update CI/CD if needed
- Consider conditional compilation

## Backend Development

### Adding a New Backend
1. Create `src/drv/cdnewbackend.c` with the driver implementation
2. Add context creation function in appropriate platform file
3. Update CMakeLists.txt to conditionally build the backend
4. Add tests in `test/`
5. Update documentation

### Backend Interface
All backends must implement the `cdContext` interface:
```c
static void cdbackend_createcanvas(cdCanvas* canvas, void* data);
static void cdbackend_killcanvas(cdCanvas* canvas);
// ... other required functions
```

## Testing

### Running Tests
```bash
cd build
make test
```

### Writing Tests
- Add test files to `test/`
- Use descriptive test names
- Test both success and failure cases
- Include platform-specific tests when relevant

### CI/CD
All pull requests are automatically tested on:
- Ubuntu Latest (GCC, X11, Cairo)
- macOS Latest (Clang, X11)  
- Windows Latest (MSVC, GDI+, Direct2D)

## Security

### Reporting Security Issues
For security-related bugs, please email the maintainers instead of opening a public issue.

### Security Considerations
- Validate all input parameters
- Check buffer bounds when processing external data
- Use safe string functions
- Consider integer overflow in calculations

## Pull Request Process

1. **Update documentation** if your changes affect the public API
2. **Ensure all tests pass** on all platforms
3. **Update CHANGELOG.md** if applicable
4. **Rebase your branch** on the latest main branch
5. **Create pull request** with:
   - Clear description of changes
   - Reference to any related issues
   - Screenshots/examples if UI-related

### PR Requirements
- [ ] Code follows style guidelines
- [ ] Tests pass on all platforms
- [ ] Documentation updated
- [ ] No breaking changes (or clearly documented)
- [ ] CMake build system updated if needed

## Release Process

1. Update version in `CMakeLists.txt` and `include/cd.h`
2. Update `CHANGELOG.md`
3. Create annotated tag: `git tag -a v5.15.0 -m "Release 5.15.0"`
4. Push tag: `git push origin v5.15.0`
5. GitHub Actions will automatically create release artifacts

## Getting Help

- Check existing [Issues](https://github.com/lispnik/tecgraf-cd/issues)
- Look at [examples](test/) and [documentation](html/)
- Join discussions in pull requests
- Reference the [original CD documentation](https://tecgraf.puc-rio.br/cd/)

## License

By contributing to CD, you agree that your contributions will be licensed under the same license as the project. See [COPYRIGHT](COPYRIGHT) for details.