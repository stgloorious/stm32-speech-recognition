#!/usr/bin/env python3

import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
from sklearn.metrics import confusion_matrix, accuracy_score

# Mapping from single-letter to full labels
label_mapping = {
    'Y': 'yes',
    'N': 'no',
    'L': 'left',
    'R': 'right',
    'U': 'up',
    'D': 'down'
}

# Read the CSV file
file_path = 'eval_data/optimized.csv'
data = pd.read_csv(file_path, header=None, names=['timestamp', 'file', 'value', 'prediction', 'label'])

# Map predictions to full names
data['prediction'] = data['prediction'].map(label_mapping)

# Check for any unmapped values
if data['prediction'].isnull().any():
    print("There are unmapped values in the prediction column.")
    print("Unmapped prediction values:", data[data['prediction'].isnull()])
    # Drop rows with unmapped values
    data.dropna(subset=['prediction'], inplace=True)

# Extract predictions and labels
predictions = data['prediction']
labels = data['label']

# Define the order of the classes
class_names = ['yes', 'no', 'left', 'right', 'up', 'down']

# Compute the confusion matrix
cm = confusion_matrix(labels, predictions, labels=class_names)

# Calculate the overall accuracy
accuracy = accuracy_score(labels, predictions) * 100
print(f'Overall accuracy: {accuracy:.2f}%')

# Plot the confusion matrix
plt.figure(figsize=(10, 7))
sns.heatmap(cm, annot=True, fmt='d', cmap='Blues', xticklabels=class_names, yticklabels=class_names)
plt.xlabel('Predicted')
plt.ylabel('Actual')
plt.title('Confusion Matrix')
plt.show()

