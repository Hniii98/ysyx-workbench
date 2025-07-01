CNT_CMD = find . -type f \( -name "*.cpp" -o -name "*.h" \) -exec cat {} + | grep -v '^\s*$$' | wc -l

count:
	@echo "Cpp non-empty lines:" 
	@$(CNT_CMD)
