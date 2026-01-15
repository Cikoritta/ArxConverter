# Arx Fatalis FTL → OBJ Converter

Конвертер 3D-моделей из формата **FTL (Arx Fatalis)** в **OBJ + MTL**.  
Предназначен для извлечения и просмотра моделей в сторонних 3D-редакторах (Blender, 3ds Max, Maya и др.).

---

<img width="534" height="507" alt="изображение" src="https://github.com/user-attachments/assets/48ba8f25-06bc-4d8e-b60d-5fc6b554ea92" />


## Возможности

- Чтение и декомпрессия `.ftl`
- Парсинг всех основных секций:
  - 3D-данные (вершины, лица, UV, нормали)
  - Группы
  - Actions
  - Selections
  - Collision spheres
  - Progressive data
  - Clothes data
- Экспорт:
  - `OBJ` (геометрия, UV, нормали)
  - `MTL` (материалы + текстуры)

---

## Использование

```bash
ArxConverter.exe <file.ftl> [output.obj]
