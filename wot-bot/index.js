require('dotenv').config();

const tmi = require('tmi.js');
const Particle = require('particle-api-js');

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

      client.say(target, `Brandon has ${numOfDevices} device online.`);
    } catch (err) {
      handleError(err);
    }
  },
  '!chase': async (target) => {
    try {
      let ret;

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

      client.say(target, `The lights are chasing each other!`);
    } catch (err) {
      handleError(err);
    }
  },
  '!dice': (target) => {
    const num = rollDice();
    client.say(target, `You rolled a ${num}`);
  },
  '!chase-stop': (target) => { },
  '!chase-color': (target) => { },
  '!token': (target) => {
    client.say(target, 'LUL brando92Cyanpanda brando92Cyanpanda LUL');
  },
  'default': (target, commandName) => {
    console.log(`command not found: ${commandName}`);
  }
}

// Called every time a message comes in
async function onMessageHandler(target, context, msg, self) {
  if (self) { return; } // Ignore messages from the bot

  // Remove whitespace from chat message
  const commandName = msg.trim();

  // Determine if this command is valid and pass to the commands fn object
  if (commandName.startsWith('!') && commands[commandName]) {
    return commands[commandName](target);
  } else {
    return commands['default'](target, commandName);
  }
}

async function getOnlineDevices() {
  let deviceList = await particle.listDevices({ auth: token });

  let deviceCount = deviceList.body.length;

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

function handleError(err) {
  client.say(target, 'Brandon wrote crap code.');
  console.log(err);
}