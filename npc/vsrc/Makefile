CNT_CMD = find . -type f \( -name "*.v" -o -name "*.vh" \) -exec cat {} + | grep -v '^\s*$$' | wc -l

count:
	@echo "Verilog non-empty lines:" 
	@$(CNT_CMD)
