for file in *.bin; do
    base_name="${file%.bin}" 
    if [ -f "${base_name}.res" ]; then
        echo "=== Testing $file ==="
        ./riscv_sim "$file"
        if diff "${file}.regdump.bin" "${base_name}.res" >/dev/null; then
            echo "✓ PASS: $file"
        else
            echo "✗ FAIL: $file"
        fi
        echo ""
    fi
done
