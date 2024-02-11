
GLSLC:=glslangValidator
GLSLC_FLAGS:= -V
SHADERS = $(wildcard shaders/*.frag shaders/*.vert)
SPIRV_SHADERS = $(addsuffix .spv, $(SHADERS))

%.frag.spv: %.frag
	$(GLSLC) $(GLSLC_FLAGS) $^ -o $@
%.vert.spv: %.vert
	$(GLSLC) $(GLSLC_FLAGS) $^ -o $@

.PHONY: shader
shader: $(SPIRV_SHADERS)


.PHONY: debug
.PHONY: release
.PHONY: clean
.PHONY: MESSAGE

MESSAGE:
	@echo [Log] Building SKVMOIP Shaders

debug: MESSAGE shader
	@echo [Log] PlayVk Shaders have been built successfully
	
release: MESSAGE shader
	@echo [Log] PlayVk Shaders have been built successfully

clean:
	$(RM) $(subst /,\, $(SPIRV_SHADERS))
	@echo [Log] SKVMOIP Shader have been cleaned successfully
