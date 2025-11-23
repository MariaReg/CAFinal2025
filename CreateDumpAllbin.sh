for file in *.bin; do
    if [[ "$file" != *".regdump.bin" ]]; then
        echo "Running: $file"
        ./riscv_sim "$file"
        echo "---"
    fi
done
