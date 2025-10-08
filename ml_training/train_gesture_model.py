"""
OpenMuscle Gesture Recognition - ML Training Pipeline

This script trains a gesture recognition model using data collected from
the OpenMuscle FlexGrid wristband.

Data Format (CSV):
- timestamp, label, S0_0...S3_14 (60 sensors), accel_x, accel_y, accel_z, gyro_x, gyro_y, gyro_z

Usage:
    python train_gesture_model.py --data collected_data.csv --output model.tflite

Author: OpenMuscle Community
License: MIT
"""

import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
from sklearn.model_selection import train_test_split
from sklearn.preprocessing import StandardScaler, LabelEncoder
from sklearn.ensemble import RandomForestClassifier
from sklearn.metrics import classification_report, confusion_matrix, accuracy_score
import tensorflow as tf
from tensorflow import keras
from tensorflow.keras import layers
import argparse
import joblib
import os

# ===== CONFIGURATION =====
WINDOW_SIZE = 50  # Number of samples per gesture (1 second at 50Hz)
STRIDE = 25  # Overlap between windows
NUM_SENSORS = 60
NUM_IMU_FEATURES = 6
TOTAL_FEATURES = NUM_SENSORS + NUM_IMU_FEATURES

# ===== DATA LOADING & PREPROCESSING =====

def load_and_preprocess_data(csv_path):
    """
    Load CSV data and preprocess for ML training
    """
    print(f"Loading data from {csv_path}...")
    df = pd.read_csv(csv_path)
    
    print(f"Total samples: {len(df)}")
    print(f"Gesture labels: {df['label'].unique()}")
    print(f"Samples per gesture:\n{df['label'].value_counts()}\n")
    
    # Extract features and labels
    sensor_cols = [f"S{row}_{col}" for row in range(4) for col in range(15)]
    imu_cols = ['accel_x', 'accel_y', 'accel_z', 'gyro_x', 'gyro_y', 'gyro_z']
    
    X = df[sensor_cols + imu_cols].values
    y = df['label'].values
    
    print(f"Feature matrix shape: {X.shape}")
    
    return X, y, df['label'].unique()


def create_sliding_windows(X, y, window_size=WINDOW_SIZE, stride=STRIDE):
    """
    Create sliding windows from continuous data
    """
    windows = []
    labels = []
    
    # Group by consecutive labels to avoid mixing gestures
    current_label = y[0]
    segment_start = 0
    
    for i in range(1, len(y)):
        if y[i] != current_label or i == len(y) - 1:
            # Extract windows from this segment
            segment_end = i
            segment_X = X[segment_start:segment_end]
            
            # Create sliding windows
            for start in range(0, len(segment_X) - window_size + 1, stride):
                window = segment_X[start:start + window_size]
                windows.append(window)
                labels.append(current_label)
            
            # Move to next segment
            segment_start = i
            current_label = y[i]
    
    windows = np.array(windows)
    labels = np.array(labels)
    
    print(f"Created {len(windows)} windows of size {window_size}")
    print(f"Window shape: {windows.shape}")
    
    return windows, labels


def extract_features(windows):
    """
    Extract statistical features from raw windows
    Useful for Random Forest model
    """
    features = []
    
    for window in windows:
        # Statistical features for each sensor channel
        mean = np.mean(window, axis=0)
        std = np.std(window, axis=0)
        min_val = np.min(window, axis=0)
        max_val = np.max(window, axis=0)
        range_val = max_val - min_val
        
        # Combine all features
        feature_vector = np.concatenate([mean, std, range_val])
        features.append(feature_vector)
    
    features = np.array(features)
    print(f"Extracted features shape: {features.shape}")
    
    return features


# ===== MODEL ARCHITECTURES =====

def build_random_forest_model(X_train, y_train):
    """
    Build and train Random Forest classifier
    Fast training, good baseline performance
    """
    print("\n=== Training Random Forest Model ===")
    
    model = RandomForestClassifier(
        n_estimators=200,
        max_depth=20,
        min_samples_split=5,
        min_samples_leaf=2,
        random_state=42,
        n_jobs=-1,
        verbose=1
    )
    
    model.fit(X_train, y_train)
    
    # Feature importance
    feature_importance = model.feature_importances_
    print(f"\nTop 10 most important features:")
    top_indices = np.argsort(feature_importance)[-10:][::-1]
    for idx in top_indices:
        print(f"  Feature {idx}: {feature_importance[idx]:.4f}")
    
    return model


def build_cnn_model(input_shape, num_classes):
    """
    Build CNN model for time-series classification
    Better accuracy, requires more training time
    """
    print("\n=== Building CNN Model ===")
    
    model = keras.Sequential([
        # First conv block
        layers.Conv1D(64, 3, activation='relu', input_shape=input_shape),
        layers.BatchNormalization(),
        layers.MaxPooling1D(2),
        layers.Dropout(0.3),
        
        # Second conv block
        layers.Conv1D(128, 3, activation='relu'),
        layers.BatchNormalization(),
        layers.MaxPooling1D(2),
        layers.Dropout(0.3),
        
        # Third conv block
        layers.Conv1D(256, 3, activation='relu'),
        layers.BatchNormalization(),
        layers.GlobalAveragePooling1D(),
        
        # Dense layers
        layers.Dense(128, activation='relu'),
        layers.Dropout(0.5),
        layers.Dense(64, activation='relu'),
        layers.Dropout(0.3),
        
        # Output layer
        layers.Dense(num_classes, activation='softmax')
    ])
    
    model.compile(
        optimizer=keras.optimizers.Adam(learning_rate=0.001),
        loss='sparse_categorical_crossentropy',
        metrics=['accuracy']
    )
    
    print(model.summary())
    
    return model


# ===== TRAINING & EVALUATION =====

def train_and_evaluate(X_train, X_test, y_train, y_test, label_encoder, 
                       model_type='random_forest', output_dir='output'):
    """
    Train model and evaluate performance
    """
    os.makedirs(output_dir, exist_ok=True)
    
    if model_type == 'random_forest':
        # Extract features for Random Forest
        X_train_features = extract_features(X_train)
        X_test_features = extract_features(X_test)
        
        # Scale features
        scaler = StandardScaler()
        X_train_scaled = scaler.fit_transform(X_train_features)
        X_test_scaled = scaler.transform(X_test_features)
        
        # Train model
        model = build_random_forest_model(X_train_scaled, y_train)
        
        # Predictions
        y_pred = model.predict(X_test_scaled)
        
        # Save model
        joblib.dump(model, os.path.join(output_dir, 'gesture_model_rf.pkl'))
        joblib.dump(scaler, os.path.join(output_dir, 'scaler.pkl'))
        
    elif model_type == 'cnn':
        # Build CNN model
        num_classes = len(np.unique(y_train))
        input_shape = (X_train.shape[1], X_train.shape[2])
        
        model = build_cnn_model(input_shape, num_classes)
        
        # Callbacks
        callbacks = [
            keras.callbacks.EarlyStopping(
                monitor='val_loss',
                patience=10,
                restore_best_weights=True
            ),
            keras.callbacks.ReduceLROnPlateau(
                monitor='val_loss',
                factor=0.5,
                patience=5,
                min_lr=1e-6
            ),
            keras.callbacks.ModelCheckpoint(
                os.path.join(output_dir, 'best_model.h5'),
                monitor='val_accuracy',
                save_best_only=True
            )
        ]
        
        # Train
        history = model.fit(
            X_train, y_train,
            validation_data=(X_test, y_test),
            epochs=100,
            batch_size=32,
            callbacks=callbacks,
            verbose=1
        )
        
        # Plot training history
        plot_training_history(history, output_dir)
        
        # Predictions
        y_pred_probs = model.predict(X_test)
        y_pred = np.argmax(y_pred_probs, axis=1)
    
    # Evaluate
    accuracy = accuracy_score(y_test, y_pred)
    print(f"\n=== Model Performance ===")
    print(f"Test Accuracy: {accuracy:.4f} ({accuracy*100:.2f}%)")
    
    # Classification report
    print("\nClassification Report:")
    gesture_names = label_encoder.classes_
    print(classification_report(y_test, y_pred, target_names=gesture_names))
    
    # Confusion matrix
    cm = confusion_matrix(y_test, y_pred)
    plot_confusion_matrix(cm, gesture_names, output_dir)
    
    return model


def plot_training_history(history, output_dir):
    """
    Plot training and validation accuracy/loss
    """
    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(12, 4))
    
    # Accuracy
    ax1.plot(history.history['accuracy'], label='Train')
    ax1.plot(history.history['val_accuracy'], label='Validation')
    ax1.set_title('Model Accuracy')
    ax1.set_xlabel('Epoch')
    ax1.set_ylabel('Accuracy')
    ax1.legend()
    ax1.grid(True)
    
    # Loss
    ax2.plot(history.history['loss'], label='Train')
    ax2.plot(history.history['val_loss'], label='Validation')
    ax2.set_title('Model Loss')
    ax2.set_xlabel('Epoch')
    ax2.set_ylabel('Loss')
    ax2.legend()
    ax2.grid(True)
    
    plt.tight_layout()
    plt.savefig(os.path.join(output_dir, 'training_history.png'), dpi=150)
    plt.close()


def plot_confusion_matrix(cm, class_names, output_dir):
    """
    Plot confusion matrix heatmap
    """
    plt.figure(figsize=(10, 8))
    sns.heatmap(cm, annot=True, fmt='d', cmap='Blues', 
                xticklabels=class_names, yticklabels=class_names)
    plt.title('Confusion Matrix')
    plt.ylabel('True Label')
    plt.xlabel('Predicted Label')
    plt.tight_layout()
    plt.savefig(os.path.join(output_dir, 'confusion_matrix.png'), dpi=150)
    plt.close()


# ===== TFLITE CONVERSION =====

def convert_to_tflite(model, output_path, quantize=True):
    """
    Convert Keras model to TensorFlow Lite format for ESP32 deployment
    """
    print("\n=== Converting to TensorFlow Lite ===")
    
    converter = tf.lite.TFLiteConverter.from_keras_model(model)
    
    if quantize:
        print("Applying post-training quantization (int8)...")
        converter.optimizations = [tf.lite.Optimize.DEFAULT]
        converter.target_spec.supported_types = [tf.int8]
    
    tflite_model = converter.convert()
    
    # Save model
    with open(output_path, 'wb') as f:
        f.write(tflite_model)
    
    print(f"TFLite model saved to: {output_path}")
    print(f"Model size: {len(tflite_model) / 1024:.2f} KB")
    
    # Test inference
    test_tflite_model(tflite_model)


def test_tflite_model(tflite_model):
    """
    Test TFLite model inference
    """
    interpreter = tf.lite.Interpreter(model_content=tflite_model)
    interpreter.allocate_tensors()
    
    input_details = interpreter.get_input_details()
    output_details = interpreter.get_output_details()
    
    print("\nTFLite Model Details:")
    print(f"Input shape: {input_details[0]['shape']}")
    print(f"Input dtype: {input_details[0]['dtype']}")
    print(f"Output shape: {output_details[0]['shape']}")
    print(f"Output dtype: {output_details[0]['dtype']}")


# ===== MAIN PIPELINE =====

def main():
    parser = argparse.ArgumentParser(description='Train gesture recognition model')
    parser.add_argument('--data', type=str, required=True, help='Path to CSV data file')
    parser.add_argument('--model_type', type=str, default='random_forest', 
                       choices=['random_forest', 'cnn'], 
                       help='Model architecture to use')
    parser.add_argument('--output_dir', type=str, default='output', 
                       help='Output directory for models')
    parser.add_argument('--tflite', action='store_true', 
                       help='Convert to TFLite (CNN only)')
    
    args = parser.parse_args()
    
    # Load data
    X, y, gesture_names = load_and_preprocess_data(args.data)
    
    # Encode labels
    label_encoder = LabelEncoder()
    y_encoded = label_encoder.fit_transform(y)
    
    # Create windows
    X_windows, y_windows = create_sliding_windows(X, y_encoded)
    
    # Train/test split
    X_train, X_test, y_train, y_test = train_test_split(
        X_windows, y_windows, test_size=0.2, random_state=42, stratify=y_windows
    )
    
    print(f"\nTrain set: {len(X_train)} samples")
    print(f"Test set: {len(X_test)} samples")
    
    # Train and evaluate
    model = train_and_evaluate(
        X_train, X_test, y_train, y_test, 
        label_encoder, args.model_type, args.output_dir
    )
    
    # Save label encoder
    joblib.dump(label_encoder, os.path.join(args.output_dir, 'label_encoder.pkl'))
    
    # Convert to TFLite if requested (CNN only)
    if args.tflite and args.model_type == 'cnn':
        tflite_path = os.path.join(args.output_dir, 'gesture_model.tflite')
        convert_to_tflite(model, tflite_path, quantize=True)
    
    print("\n✓ Training complete!")
    print(f"Model saved to: {args.output_dir}/")


if __name__ == '__main__':
    main()
