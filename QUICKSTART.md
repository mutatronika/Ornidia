# Ornidia - Guía Rápida de Pruebas

## Ejecutar Verificación de Integridad

```bash
# Hacer el script ejecutable (solo la primera vez)
chmod +x verify_integrity.sh

# Ejecutar las pruebas
./verify_integrity.sh
```

## Salida Esperada

Si todo está correcto:
```
✓ ALL TESTS PASSED! ✓
Project integrity verified successfully!
```

## Pruebas Incluidas

- ✓ 76 pruebas automatizadas
- ✓ Validación de estructura del proyecto
- ✓ Verificación de sintaxis de sketches
- ✓ Análisis de dependencias
- ✓ Validación de archivos web
- ✓ Comprobación de documentación
- ✓ Integridad del repositorio Git

## Ver Resultados Detallados

```bash
cat /tmp/ornidia_test_results.txt
```

## Documentación Completa

- `TESTING.md` - Guía completa de pruebas
- `LIBRARIES.md` - Dependencias y librerías requeridas
- `README.md` - Documentación del proyecto

## Requisitos del Sistema

- Bash shell (Linux/macOS/Git Bash en Windows)
- Git
- Permisos de ejecución en el directorio

## Solución Rápida de Problemas

### Script no ejecuta
```bash
chmod +x verify_integrity.sh
```

### Tests fallan
1. Ver el reporte: `cat /tmp/ornidia_test_results.txt`
2. Revisar qué test falló
3. Consultar `TESTING.md` para soluciones

---

Para más información, consulta `TESTING.md`
