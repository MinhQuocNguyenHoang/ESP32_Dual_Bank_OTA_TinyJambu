# Model to C++ Converter for Embedded IoT

This directory contains scripts to convert trained Machine Learning models (Linear Regression, Gradient Boosting, etc.) into standalone C++ header files (`.h`) for microcontrollers like ESP32 or STM32.

## Usage

### 1. Convert Latest Model Automatically
```bash
python software/convert_to_cpp/export_model_to_cpp.py
```
This automatically finds the latest model in `models/production/` and generates the corresponding C++ header file in `firmware/`.

### 2. Convert Specific Model File
```bash
python software/convert_to_cpp/export_model_to_cpp.py --model_path models/production/GradientBoostingRegressor_20260821_143239.joblib --output firmware/gradient_boosting_model.h
```

## How it Works
- **Linear Regression**: Generates C++ constants for coefficients and an inline evaluation function.
- **Gradient Boosting**: Traverses decision trees and exports them into optimized nested `if-else` C++ functions (`predict_gb_tree_0`, `predict_gb_tree_1`, ...), then sums their weighted predictions.
