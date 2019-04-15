require('dotenv').config();

const tmi = require('tmi.js');
const Particle = require('particle-api-js');
const tinycolor = require('tinycolor2');

const particle = new Particle();
const token = process.env.PARTICLE_TOKEN;
const WOTController = process.env.WOT_CONTROLLER_ID;

const opts = {
  identity: {
    username: 'cyanpandabot',
    password: process.env.TWITCH_TOKEN
  },
  channels: ['brandonsatrom']
};

const client = new tmi.client(opts);

client.on('message', onMessageHandler);
client.on('connected', onConnectedHandler);

client.connect();

const commands = {
  '!online': async (target) => {
    try {
      // Query device cloud for the # of devices Brandon has online
      const numOfDevices = await getOnlineDevices();

      client.say(target, `Brandon has ${numOfDevices} Particle devices online.`);
    } catch (err) {
      handleError(target, err);
    }
  },
  '!chase': async (target, context, colors) => {
    try {
      let ret;

      ret = await setChaseColors(colors);

      if (!ret) {
        client.say(target, 'Invalid color! Try passing a color value by name, hex or rgb list.');
      }

      const stripMode = await particle.getVariable({
        deviceId: WOTController,
        name: 'stripMode',
        auth: token
      });

      // If the result value is 5, chase mode is already active
      // so we don't need to set it again
      if (stripMode.body.result !== 5) {
        // Set Mode to Chase
        ret = await particle.callFunction({
          deviceId: WOTController,
          name: 'setMode',
          argument: '5',
          auth: token
        });

        // Call Chase
        ret = await particle.callFunction({
          deviceId: WOTController,
          name: 'chase',
          auth: token
        });

        if (ret.body.return_value === 1) {
          client.say(target, `Yay ${context.username}! The lights are chasing each other!`);
        } else {
          throw new Error('Got a failure code from the Particle function.')
        }
      } else {
        client.say(target, `Yay ${context.username}! You set the lights to chase in ${colors}!`);
      }
    } catch (err) {
      handleError(target, err);
    }
  },
  '!dice': (target, context) => {
    const num = rollDice();
    client.say(target, `@${context.username}, You rolled a ${num}`);
  },
  '!stop': async (target, context) => {
    try {
      let ret;

      // Set Mode to Random to turn off lights
      ret = await particle.callFunction({
        deviceId: WOTController,
        name: 'setMode',
        argument: '0',
        auth: token
      });

      if (ret.body.return_value === 1) {
        client.say(target, `lights are off!`);
      } else {
        throw new Error('Got a failure code from the Particle function.')
      }
    } catch (err) {
      handleError(target, err);
    }
  },
  '!chase-color': async (target, context, colors) => {
    try {
      const stripMode = await particle.getVariable({
        deviceId: WOTController,
        name: 'stripMode',
        auth: token
      });

      // If the result value is not 5, chase mode isn't active
      if (stripMode.body.result !== 5) {
        client.say(target, `Chase mode isn't active. Try running the !chase command with a color.`);
      } else {
        const ret = await setChaseColors(colors);

        if (ret.body.return_value === 1) {
          client.say(target, `Yay ${context.username}! You changed the light color to ${colors}`);
        } else {
          throw new Error('Got a failure code from the Particle function.')
        }
      }
    } catch (err) {
      handleError(target, err);
    }
  },
  '!breathe': async (target, context) => {
    await triggerLEDMode(target, context, '6', 'breathe');
  },
  '!rainbow': async (target, context) => {
    await triggerLEDMode(target, context, '4', 'rainbow');
  },
  '!cheers': (target, context) => { },
  '!james': (target, context) => { },
  '!token': (target, context) => {
    client.say(target, `Nice try @${context.username} LUL brando92Cyanpanda brando92Cyanpanda LUL`);
  },
  'default': (target, commandName) => {
    // console.log(`command not found: ${commandName}`);
  }
}

// Called every time a message comes in
async function onMessageHandler(target, context, msg, self) {
  if (self) { return; } // Ignore messages from the bot

  // Remove whitespace from chat message
  const commandName = msg.trim();

  // Determine if this command is valid and pass to the commands fn object
  if (commandName.startsWith('!') && commands[commandName]) {
    return commands[commandName](target, context);
  } else if (commandName.startsWith('!chase')) {
    const command = commandName.split(' ')[0];
    const colors = commandName.substring(commandName.indexOf(' ') + 1);

    console.log("COMM: ", command);
    console.log("COLORS: ", colors);

    return commands[command](target, context, colors);
  } else {
    return commands['default'](target, commandName);
  }
}

async function getOnlineDevices() {
  let deviceList = await particle.listDevices({ auth: token });

  let deviceCount = deviceList.body.filter((device => device.connected)).length;

  return deviceCount;
}

// Function called when the "dice" command is issued
function rollDice() {
  const sides = 6;
  return Math.floor(Math.random() * sides) + 1;
}

// Called every time the bot connects to Twitch chat
function onConnectedHandler(addr, port) {
  console.log(`* Connected to ${addr}:${port}`);
}

function handleError(target, err) {
  client.say(target, 'Something went wrong! Probably not your fault, but it could be...');
  console.log(err);
}

async function setChaseColors(colors) {
  let ret;

  if (colors) {
    let colorList, color;

    // if colors is a space or comma-separated list, split for RGB
    if (colors.startsWith('#')) { // if color is Hex, convert to RGB
      color = tinycolor(colors);

      if (color.isValid()) {
        colorList = hexToRgb(color.toHexString());

        if (!colorList) return null;
      } else {
        return null;
      }
    } else if (colors.match(/[,\s]/g)) {
      colorList = colors.split(',').length > 1
        ? colors.split(',')
        : colors.split(' ');
    } else { // Parse named color
      color = tinycolor(colors);

      if (color.isValid()) {
        console.log('HEX: ', color.toHexString());

        colorList = hexToRgb(color.toHexString());

        if (!colorList) return null;
      } else {
        return null;
      }
    }

    return await particle.callFunction({
      deviceId: WOTController,
      name: 'setColors',
      argument: `{ "red": ${colorList[0]}, "green": ${colorList[1]}, "blue": ${colorList[2]}}`,
      auth: token
    });
  }

  return null;
}

function hexToRgb(hex) {
  var result = /^#?([a-f\d]{2})([a-f\d]{2})([a-f\d]{2})$/i.exec(hex);

  return result ? [
    parseInt(result[1], 16),
    parseInt(result[2], 16),
    parseInt(result[3], 16)
  ] : null;
}

async function triggerLEDMode(target, context, mode, name) {
  try {
    let ret;

    // Set Mode to Random to turn off lights
    ret = await particle.callFunction({
      deviceId: WOTController,
      name: 'setMode',
      argument: mode,
      auth: token
    });

    if (ret.body.return_value === 1) {
      client.say(target, `Yay ${context.username}! You triggered ${name} mode!`);
    } else {
      throw new Error('Got a failure code from the Particle function.')
    }
  } catch (err) {
    handleError(err);
  }
}