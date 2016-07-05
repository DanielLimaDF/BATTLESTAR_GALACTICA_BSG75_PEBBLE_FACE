module.exports = [
  {
    "type": "heading",
    "defaultValue": "BSG75 Watchface Configuration"
  },
  {
    "type": "text",
    "defaultValue": "So say we all!"
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Settings"
      },
      {
        "type": "toggle",
        "messageKey": "showBattery",
        "label": "Show battery",
        "defaultValue": true
      },
      {
        "type": "toggle",
        "messageKey": "showDate",
        "label": "Show date",
        "defaultValue": true
      },
      {
        "type": "toggle",
        "messageKey": "showSteps",
        "label": "Show steps",
        "defaultValue": true,
        "capabilities": ["HEALTH"],
      }
    ]
  },
  {
    "type": "submit",
    "defaultValue": "Save Settings"
  }
];