"""
Model to C++ Header Exporter for Embedded IoT / ESP32 Firmware
Author: Antigravity Assistant & Raz Rizkov

This script loads trained machine learning models (.joblib or .csv)
and generates clean C/C++ header files (.h) containing native prediction functions.
Supported models:
- Linear Regression
- Gradient Boosting Regressor
"""

import os
import glob
import argparse
import joblib
import pandas as pd
import numpy as np


def export_linear_regression(model, feature_names, output_path):
    """
    Export Linear Regression model to C++ header file
    """
    intercept = model.intercept_ if hasattr(model, 'intercept_') else 0.0
    coefs = model.coef_ if hasattr(model, 'coef_') else []

    with open(output_path, 'w') as f:
        f.write("#ifndef LINEAR_REGRESSION_MODEL_H\n")
        f.write("#define LINEAR_REGRESSION_MODEL_H\n\n")
        f.write("// Auto-generated Linear Regression Coefficients for Embedded IoT\n")
        f.write(f"const float INTERCEPT = {intercept:.6f}f;\n")

        for idx, (name, coef) in enumerate(zip(feature_names, coefs)):
            f.write(f"const float COEF_{name.upper()} = {coef:.6f}f;\n")

        f.write("\ninline float predict_linear_regression(const float* features) {\n")
        f.write("    float pred = INTERCEPT;\n")
        for idx, coef in enumerate(coefs):
            f.write(f"    pred += COEF_{feature_names[idx].upper()} * features[{idx}];\n")
        f.write("    return pred;\n")
        f.write("}\n\n#endif // LINEAR_REGRESSION_MODEL_H\n")

    print(f"[SUCCESS] Exported Linear Regression model to: {output_path}")


def export_gradient_boosting(model, feature_names, output_path):
    """
    Export GradientBoostingRegressor to C++ header file using nested decision trees
    """
    init_value = model.init_.constant_[0][0] if hasattr(model.init_, 'constant_') else 0.0
    learning_rate = model.learning_rate

    cpp_code = "#ifndef GRADIENT_BOOSTING_MODEL_H\n"
    cpp_code += "#define GRADIENT_BOOSTING_MODEL_H\n\n"
    cpp_code += "// Auto-generated Gradient Boosting Trees for Embedded IoT\n"
    cpp_code += f"const float GB_INIT_VAL = {init_value:.6f}f;\n"
    cpp_code += f"const float GB_LEARNING_RATE = {learning_rate:.6f}f;\n\n"

    estimators = model.estimators_.flatten()
    for i, tree in enumerate(estimators):
        t = tree.tree_
        cpp_code += f"inline float predict_gb_tree_{i}(const float* x) {{\n"

        def recurse(node):
            if t.children_left[node] == -1:  # Leaf node
                return f"    return {t.value[node][0][0]:.6f}f;\n"
            else:
                feat = t.feature[node]
                thresh = t.threshold[node]
                left = recurse(t.children_left[node])
                right = recurse(t.children_right[node])
                return f"    if (x[{feat}] <= {thresh:.6f}f) {{\n{left}    }} else {{\n{right}    }}\n"

        cpp_code += recurse(0)
        cpp_code += "}\n\n"

    cpp_code += "inline float predict_gradient_boosting(const float* x) {\n"
    cpp_code += "    float pred = GB_INIT_VAL;\n"
    for i in range(len(estimators)):
        cpp_code += f"    pred += GB_LEARNING_RATE * predict_gb_tree_{i}(x);\n"
    cpp_code += "    return pred;\n"
    cpp_code += "}\n\n#endif // GRADIENT_BOOSTING_MODEL_H\n"

    with open(output_path, 'w') as f:
        f.write(cpp_code)

    print(f"[SUCCESS] Exported {len(estimators)} Gradient Boosting trees to: {output_path}")


def main():
    parser = argparse.ArgumentParser(description="Export ML models to C++ headers")
    parser.add_argument("--model_path", type=str, default=None,
                        help="Path to .joblib or .csv model file.")
    parser.add_argument("--output", type=str, default=None,
                        help="Output .h file path.")
    args = parser.parse_args()

    model_path = args.model_path
    if not model_path:
        joblib_files = sorted(glob.glob("models/production/*.joblib"))
        if joblib_files:
            model_path = joblib_files[-1]
        else:
            csv_files = sorted(glob.glob("models/production/*.csv"))
            if csv_files:
                model_path = csv_files[-1]
            else:
                raise FileNotFoundError("No trained model files (.joblib or .csv) found in models/production/")

    print(f"Loading model from: {model_path}")
    feature_names = ['ratio', 'variability', 'slope', 'pulse_rate', 'acdc_ratio']

    output_path = args.output
    if not output_path:
        if "GradientBoosting" in model_path:
            output_path = "firmware/gradient_boosting_model.h"
        else:
            output_path = "firmware/model_coefficients.h"

    if model_path.endswith('.joblib'):
        model = joblib.load(model_path)
        model_type = type(model).__name__
        print(f"Model Type: {model_type}")

        if model_type == "GradientBoostingRegressor":
            export_gradient_boosting(model, feature_names, output_path)
        elif model_type == "LinearRegression":
            export_linear_regression(model, feature_names, output_path)
        else:
            raise NotImplementedError(f"Export for model type '{model_type}' is not implemented yet.")
    elif model_path.endswith('.csv'):
        df = pd.read_csv(model_path)
        intercept = df[df['feature'] == 'intercept']['coefficient'].values[0] if 'intercept' in df['feature'].values else 0.0
        coef_df = df[df['feature'] != 'intercept']

        with open(output_path, 'w') as f:
            f.write("#ifndef MODEL_COEFFICIENTS_H\n#define MODEL_COEFFICIENTS_H\n\n")
            f.write(f"const float INTERCEPT = {intercept:.6f}f;\n")
            for idx, row in coef_df.iterrows():
                f.write(f"const float COEF_{row['feature'].upper()} = {row['coefficient']:.6f}f;\n")
            f.write("\n#endif\n")
        print(f"[SUCCESS] Exported CSV coefficients to: {output_path}")


if __name__ == "__main__":
    main()
